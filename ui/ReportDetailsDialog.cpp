#include "ReportDetailsDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSet>
#include <QVBoxLayout>
#include <QPushButton>

#include "QtBridge.h"
#include "../core/DynamicArray.h"

using namespace pbl2;
using namespace pbl2::bridge;

using namespace std;

namespace {

QString formatDate(const QDate &value) {
    if (!value.isValid()) {
        return QObject::tr("Không rõ");
    }
    return value.toString(Qt::ISODate);
}

QString formatDateTime(const QDateTime &value) {
    if (!value.isValid()) {
        return QObject::tr("Không rõ");
    }
    return value.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
}

QString detailSecondaryActionStyle() {
    return QStringLiteral(
        "QPushButton { background: #64748b; color: #ffffff; border: none; border-radius: 8px; padding: 6px 14px; font-weight: 600; }"
        "QPushButton:hover { background: #475569; }"
        "QPushButton:pressed { background: #334155; }");
}

QLabel *makeCaption(const QString &text, QWidget *parent) {
    auto *label = new QLabel(text, parent);
    label->setMinimumWidth(120);
    return label;
}

QLabel *makeValueLabel(const QString &text, QWidget *parent) {
    auto *label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    return label;
}

struct AffectedBookRow {
    QString id;
    QString title;
    int count{0};
};

core::DynamicArray<AffectedBookRow> parseAffectedBooks(const QString &raw, const core::DynamicArray<model::Book> &books) {
    core::DynamicArray<AffectedBookRow> rows;
    QString normalized = raw;
    normalized.replace('\n', ';');
    normalized.replace(',', ';');
    QString current;

    auto appendEntry = [&](const QString &token) {
        if (token.isEmpty()) return;
        const int sep = token.indexOf(':');
        const QString id = (sep >= 0 ? token.left(sep) : token).trimmed();
        const QString countStr = sep >= 0 ? token.mid(sep + 1).trimmed() : QStringLiteral("1");
        bool ok = true;
        const int count = countStr.isEmpty() ? 1 : countStr.toInt(&ok);
        if (!ok || count <= 0 || id.isEmpty()) return;

        const QString canonicalId = id.toUpper();
        int existing = -1;
        for (int i = 0; i < rows.size(); ++i) {
            if (rows[i].id.compare(canonicalId, Qt::CaseInsensitive) == 0) {
                existing = i;
                break;
            }
        }
        if (existing >= 0) {
            rows[existing].count += count;
            return;
        }
        AffectedBookRow row;
        row.id = canonicalId;
        row.count = count;
        for (const auto &book : books) {
            if (toQString(book.getId()).compare(canonicalId, Qt::CaseInsensitive) == 0) {
                row.title = toQString(book.getTitle());
                break;
            }
        }
        rows.append(row);
    };

    for (const QChar ch : normalized) {
        if (ch == QChar(';')) {
            const QString trimmed = current.trimmed();
            appendEntry(trimmed);
            current.clear();
        } else {
            current.append(ch);
        }
    }
    appendEntry(current.trimmed());
    return rows;
}

core::DynamicArray<QPair<QString, QString>> parseNoteFields(const QString &raw) {
    core::DynamicArray<QPair<QString, QString>> rows;
    if (raw.trimmed().isEmpty()) return rows;
    const QStringList tokens = raw.split('|');
    for (const QString &token : tokens) {
        const QString trimmed = token.trimmed();
        if (trimmed.isEmpty()) continue;
        const int sep = trimmed.indexOf(':');
        if (sep <= 0) {
            const QString key = QObject::tr("Thông tin khác");
            int existing = -1;
            for (int i = 0; i < rows.size(); ++i) {
                if (rows[i].first.compare(key, Qt::CaseInsensitive) == 0) {
                    existing = i;
                    break;
                }
            }
            if (existing >= 0) {
                rows[existing].second = QStringLiteral("%1 | %2").arg(rows[existing].second, trimmed);
            } else {
                rows.append({key, trimmed});
            }
            continue;
        }
        QString key = trimmed.left(sep).trimmed();
        QString value = trimmed.mid(sep + 1).trimmed();
        if (key.compare(QStringLiteral("Phiếu"), Qt::CaseInsensitive) == 0) {
            key = QObject::tr("Mã phiếu");
        } else if (key.compare(QStringLiteral("Mượn"), Qt::CaseInsensitive) == 0) {
            key = QObject::tr("Ngày mượn");
        }
        if (value.isEmpty()) continue;
        int existing = -1;
        for (int i = 0; i < rows.size(); ++i) {
            if (rows[i].first.compare(key, Qt::CaseInsensitive) == 0) {
                existing = i;
                break;
            }
        }
        if (existing >= 0) {
            rows[existing].second = QStringLiteral("%1 | %2").arg(rows[existing].second, value);
        } else {
            rows.append({key, value});
        }
    }
    return rows;
}

}  // namespace

namespace pbl2::ui {

ReportDetailsDialog::ReportDetailsDialog(const model::ReportRequest &report,
                                         const QString &statusText,
                                         const core::DynamicArray<model::Book> &books,
                                         QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(tr("Chi tiết báo cáo"));
    setModal(true);
    setMinimumSize(640, 560);
    setWindowIcon(QIcon(":/ui/resources/icons/report.png"));
    setStyleSheet(R"(
QDialog { background: #f8fafc; border-radius: 12px; }
QGroupBox { font-weight: bold; border-radius: 8px; }
QLineEdit, QComboBox, QSpinBox, QDateEdit, QPlainTextEdit {
    min-height: 32px; font-size: 11pt; border-radius: 10px; background: #fff;
    border: 1.5px solid #e3e8f0; padding-left: 10px;
}
QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDateEdit:focus, QPlainTextEdit:focus {
    border: 2px solid #2f6ad0; background: #f0f6ff;
}
QDialogButtonBox QPushButton, QPushButton {
    min-width: 100px; min-height: 36px; font-size: 11pt; border-radius: 10px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2f6ad0, stop:1 #6c63ff);
    color: #fff; font-weight: 500; border: none;
}
QDialogButtonBox QPushButton:hover, QPushButton:hover {
    background: #466ee6;
}
QDialogButtonBox QPushButton:disabled, QPushButton:disabled {
    background: #bfc9db; color: #fff;
}
QLabel { font-size: 11pt; }
QLabel[error="true"] { color: #dc2626; font-size: 10.5pt; padding: 6px; }
)");
    // Modern font and light background
    setStyleSheet("QDialog { background: #f8fafc; border-radius: 12px; } QGroupBox { font-weight: bold; } QLineEdit, QComboBox, QSpinBox, QDateEdit, QPlainTextEdit { min-height: 32px; font-size: 11pt; } QDialogButtonBox QPushButton { min-width: 140px; min-height: 44px; font-size: 12pt; font-weight: 700; } QLabel { font-size: 11pt; } ");

    const QString staff = bridge::toQString(report.getStaffUsername());
    const QString notes = bridge::toQString(report.getNotes()).trimmed();

    auto *form = new QFormLayout;
    form->setContentsMargins(12, 12, 12, 12);
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(10);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    form->addRow(makeCaption(tr("Nhân viên"), this), makeValueLabel(displayOrFallback(staff, tr("Không rõ")), this));
    form->addRow(makeCaption(tr("Từ ngày"), this), makeValueLabel(formatDate(bridge::toQDate(report.getFromDate())), this));
    form->addRow(makeCaption(tr("Đến ngày"), this), makeValueLabel(formatDate(bridge::toQDate(report.getToDate())), this));
    form->addRow(makeCaption(tr("Trạng thái"), this), makeValueLabel(displayOrFallback(statusText, tr("Không rõ")), this));
    form->addRow(makeCaption(tr("Ngày tạo"), this), makeValueLabel(formatDateTime(bridge::toQDateTime(report.getCreatedAt())), this));
    const auto noteFields = parseNoteFields(notes);
    QSet<QString> existingLabels{
        tr("Nhân viên"),
        tr("Từ ngày"),
        tr("Đến ngày"),
        tr("Trạng thái"),
        tr("Ngày tạo"),
    };
    for (const auto &row : noteFields) {
        if (row.first.trimmed().isEmpty()) continue;
        const QString normalized = row.first.trimmed().toLower();
        bool duplicate = false;
        for (const QString &label : existingLabels) {
            if (label.trimmed().toLower() == normalized) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;
        existingLabels.insert(row.first.trimmed());
        form->addRow(makeCaption(row.first, this), makeValueLabel(row.second, this));
    }

    const auto affectedRows = parseAffectedBooks(bridge::toQString(report.getAffectedBooks()), books);
    QString affectedText;
    for (int i = 0; i < affectedRows.size(); ++i) {
        const auto &row = affectedRows[i];
        const QString title = row.title.trimmed();
        const QString name = title.isEmpty() ? row.id : QStringLiteral("%1 (%2)").arg(title, row.id);
        if (!affectedText.isEmpty()) affectedText.append('\n');
        affectedText.append(QStringLiteral("• %1 x%2").arg(name, QString::number(row.count)));
    }
    if (affectedText.isEmpty()) {
        affectedText = tr("(Không có sách được liên kết)");
    }
    form->addRow(makeCaption(tr("Chi tiết sách"), this), makeValueLabel(affectedText, this));

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttonBox->setCenterButtons(true);
    if (auto *closeBtn = buttonBox->button(QDialogButtonBox::Close)) {
        closeBtn->setMinimumSize(120, 38);
        closeBtn->setMaximumWidth(160);
        closeBtn->setDefault(true);
        closeBtn->setStyleSheet(detailSecondaryActionStyle());
    }
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ReportDetailsDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addLayout(form);
    layout->addWidget(buttonBox, 0, Qt::AlignRight);
}

QString ReportDetailsDialog::displayOrFallback(const QString &value, const QString &fallback) {
    return value.trimmed().isEmpty() ? fallback : value;
}

}  // namespace pbl2::ui

#include "LoanDialog.h"

#include "QtBridge.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QCompleter>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QVariant>
#include <QVBoxLayout>

using namespace std;

namespace {

using pbl2::model::Book;
using pbl2::model::Loan;
using pbl2::model::LoanItem;
using pbl2::model::Reader;

void setupSearchableCombo(QComboBox *combo, const QString &placeholder = QString()) {
    if (!combo) return;
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    if (QLineEdit *edit = combo->lineEdit()) {
        edit->setPlaceholderText(placeholder);
    }

    auto *completer = new QCompleter(combo->model(), combo);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    combo->setCompleter(completer);
}

QString displayReader(const Reader &reader) {
    const QString id = pbl2::bridge::toQString(reader.getId());
    const QString name = pbl2::bridge::toQString(reader.getFullName());
    return QStringLiteral("%1 - %2").arg(id, name);
}

QString displayBook(const Book &book) {
    const QString bookId = pbl2::bridge::toQString(book.getId());
    const QString title = pbl2::bridge::toQString(book.getTitle());
    return QStringLiteral("%1 - %2").arg(bookId, title);
}

QString primaryActionStyle() {
    return QStringLiteral(
        "QPushButton { background: #16a34a; color: #ffffff; border: none; border-radius: 8px; padding: 6px 14px; font-weight: 600; }"
        "QPushButton:hover { background: #15803d; }"
        "QPushButton:pressed { background: #166534; }");
}

QString dangerActionStyle() {
    return QStringLiteral(
        "QPushButton { background: #dc2626; color: #ffffff; border: none; border-radius: 8px; padding: 6px 14px; font-weight: 600; }"
        "QPushButton:hover { background: #b91c1c; }"
        "QPushButton:pressed { background: #991b1b; }");
}

}

namespace pbl2::ui {

    LoanDialog::LoanDialog(const core::DynamicArray<Reader> &readers,
                           const core::DynamicArray<Book> &books,
                           const int maxBorrowDays,
                           const QString &staffUsername,
                           QWidget *parent)
        : QDialog(parent),
          readers(readers),
          books(books) {
        setWindowTitle(tr("Phiếu mượn trả"));
        setModal(true);
        setWindowIcon(QIcon(":/ui/resources/icons/loan.png"));
        const QFont font("Segoe UI", 11);
        setFont(font);
        // Dùng chung style đơn giản với Thong tin ban doc
        setStyleSheet("QDialog { background: #f8fafc; border-radius: 12px; } "
            "QGroupBox { font-weight: bold; } "
            "QLineEdit, QComboBox, QSpinBox, QDateEdit, QPlainTextEdit { "
            "  min-height: 32px; font-size: 11pt; } "
            "QDialogButtonBox QPushButton { "
            "  min-width: 90px; min-height: 32px; font-size: 11pt; } "
            "QLabel { font-size: 11pt; } ");

        this->maxBorrowDays = maxBorrowDays > 0 ? maxBorrowDays : 14;

        loanIdEdit = new QLineEdit(this);
        readerCombo = new QComboBox(this);
        itemsList = new QListWidget(this);
        addItemButton = new QPushButton(tr("Thêm sách"), this);
        editItemButton = new QPushButton(tr("Sửa"), this);
        removeItemButton = new QPushButton(tr("Xóa"), this);
        staffEdit = new QLineEdit(this);
        staffEdit->setReadOnly(true);

        for (const auto &reader : readers) {
            readerCombo->addItem(displayReader(reader), QVariant(bridge::toQString(reader.getId())));
        }

        setupSearchableCombo(readerCombo, tr("Tìm bạn đọc"));

        readerCombo->setCurrentIndex(-1);
        readerCombo->clearEditText();

        staffEdit->setText(staffUsername);

        borrowDateEdit = new QDateEdit(this);
        borrowDateEdit->setCalendarPopup(true);
        borrowDateEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
        borrowDateEdit->setDate(bridge::currentDate());

        itemsList->setSelectionMode(QAbstractItemView::SingleSelection);
        itemsList->setMinimumHeight(150);

        errorLabel = new QLabel(this);
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setVisible(false);

        auto *formGroup = new QGroupBox(tr("Chi tiết phiếu mượn"), this);
        auto *form = new QFormLayout;
        form->setContentsMargins(12, 12, 12, 12);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->addRow(tr("Mã phiếu"), loanIdEdit);
        form->addRow(tr("Bạn đọc"), readerCombo);
        form->addRow(tr("Nhân viên lập phiếu"), staffEdit);
        form->addRow(tr("Ngày mượn"), borrowDateEdit);

        auto *itemsPanel = new QWidget(this);
        auto *itemsLayout = new QVBoxLayout(itemsPanel);
        itemsLayout->setContentsMargins(0, 0, 0, 0);
        itemsLayout->addWidget(itemsList);
        auto *buttonRow = new QHBoxLayout;
        buttonRow->setContentsMargins(0, 0, 0, 0);
        buttonRow->addWidget(addItemButton);
        buttonRow->addWidget(editItemButton);
        buttonRow->addWidget(removeItemButton);
        buttonRow->addStretch(1);
        itemsLayout->addLayout(buttonRow);
        form->addRow(tr("Danh sách sách"), itemsPanel);
        formGroup->setLayout(form);

        connect(addItemButton, &QPushButton::clicked, this, [this]() { showItemEditor(-1); });
        connect(editItemButton, &QPushButton::clicked, this, [this]() { showItemEditor(itemsList ? itemsList->currentRow() : -1); });
        connect(removeItemButton, &QPushButton::clicked, this, [this]() {
            if (!itemsList) return;
            const int row = itemsList->currentRow();
            if (row < 0 || static_cast<size_t>(row) >= loanItems.size()) return;
            loanItems.removeAt(static_cast<core::DynamicArray<model::LoanItem>::SizeType>(row));
            refreshItemsList();
        });

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        if (auto *okBtn = buttonBox->button(QDialogButtonBox::Ok)) {
            okBtn->setStyleSheet(primaryActionStyle());
        }
        if (auto *cancelBtn = buttonBox->button(QDialogButtonBox::Cancel)) {
            cancelBtn->setStyleSheet(dangerActionStyle());
        }
        connect(buttonBox, &QDialogButtonBox::accepted, this, &LoanDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &LoanDialog::reject);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);
        layout->addWidget(formGroup);
        layout->addWidget(errorLabel);
        layout->addWidget(buttonBox);
        refreshItemsList();
        setMinimumSize(600, 500);
    }

    void LoanDialog::presetLoanId(const QString &loanId, bool lockField) {
        forceIdReadOnly = lockField;
        loanIdEdit->setText(loanId.trimmed());
        loanIdEdit->setReadOnly(editingMode || forceIdReadOnly);
    }

    void LoanDialog::presetInitialBook(const QString &bookId) {
        const QString trimmedId = bookId.trimmed();
        if (trimmedId.isEmpty()) return;

        for (const auto &book : books) {
            const QString id = bridge::toQString(book.getId());
            if (id.compare(trimmedId, Qt::CaseInsensitive) == 0) {
                model::LoanItem item;
                item.setBookId(book.getId());
                item.setQuantity(1);
                const QDate due = borrowDateEdit ? borrowDateEdit->date().addDays(maxBorrowDays) : bridge::currentDate().addDays(maxBorrowDays);
                item.setDueDate(bridge::toCoreDate(due));
                item.setReturnDate(pbl2::core::Date());
                item.setStatus(bridge::toCustomString(QStringLiteral("BORROWED")));
                item.setFine(0);
                item.setExtensionCount(0);
                // Avoid duplicates
                for (const auto &existing : loanItems) {
                    if (bridge::toQString(existing.getBookId()).compare(id, Qt::CaseInsensitive) == 0) {
                        return;
                    }
                }
                loanItems.append(item);
                refreshItemsList();
                break;
            }
        }
    }

    bool LoanDialog::showItemEditor(const int editRow) {
        QDialog dialog(this);
        dialog.setWindowTitle(editRow >= 0 ? tr("Sửa sách mượn") : tr("Thêm sách mượn"));
        auto *form = new QFormLayout(&dialog);
        form->setContentsMargins(16, 16, 16, 16);
        form->setSpacing(10);

        auto *bookCombo = new QComboBox(&dialog);
        bookCombo->setEditable(true);
        bookCombo->setInsertPolicy(QComboBox::NoInsert);
        auto *dueEdit = new QDateEdit(&dialog);
        dueEdit->setCalendarPopup(true);
        dueEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
        dueEdit->setDate(borrowDateEdit ? borrowDateEdit->date().addDays(maxBorrowDays) : bridge::currentDate().addDays(maxBorrowDays));

        // Build set of already selected books (exclude the one being edited)
        QSet<QString> taken;
        for (int i = 0; i < static_cast<int>(loanItems.size()); ++i) {
            if (i == editRow) continue;
            taken.insert(bridge::toQString(loanItems[i].getBookId()).toUpper());
        }

        const auto existingBookId = (editRow >= 0 && static_cast<size_t>(editRow) < loanItems.size())
                                        ? bridge::toQString(loanItems[editRow].getBookId())
                                        : QString();

        for (const auto &book : books) {
            const QString id = bridge::toQString(book.getId()).trimmed();
            if (taken.contains(id.toUpper())) continue;
            bookCombo->addItem(displayBook(book), QVariant(id));
        }

        if (editRow >= 0 && bookCombo->findData(existingBookId) == -1) {
            // If the original book is filtered out (shouldn't happen), add it for editing.
            bookCombo->addItem(existingBookId, QVariant(existingBookId));
        }

        const int foundIdx = existingBookId.isEmpty() ? -1 : bookCombo->findData(existingBookId);
        if (foundIdx >= 0) {
            bookCombo->setCurrentIndex(foundIdx);
        } else {
            bookCombo->setCurrentIndex(-1);
            bookCombo->clearEditText();
        }
        setupSearchableCombo(bookCombo, tr("Chọn sách"));

        if (editRow >= 0 && static_cast<size_t>(editRow) < loanItems.size()) {
            if (loanItems[editRow].getDueDate().isValid()) {
                dueEdit->setDate(bridge::toQDate(loanItems[editRow].getDueDate()));
            }
        }

        form->addRow(tr("Sách"), bookCombo);
        form->addRow(tr("Hạn trả"), dueEdit);

        auto *dlgButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        dlgButtons->button(QDialogButtonBox::Ok)->setStyleSheet(primaryActionStyle());
        dlgButtons->button(QDialogButtonBox::Cancel)->setStyleSheet(dangerActionStyle());
        form->addWidget(dlgButtons);
        QObject::connect(dlgButtons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(dlgButtons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted) return false;
        const QString selectedId = bookCombo->currentData().toString().trimmed();
        if (selectedId.isEmpty()) {
            showError(tr("Vui lòng chọn sách."));
            return false;
        }
        if (taken.contains(selectedId.toUpper())) {
            showError(tr("Mỗi phiếu chỉ chọn một lần cho mỗi sách."));
            return false;
        }

        const QDate dueDate = dueEdit->date();
        if (borrowDateEdit && dueDate < borrowDateEdit->date()) {
            showError(tr("Ngày trả phải sau ngày mượn."));
            return false;
        }

        model::LoanItem item;
        item.setBookId(bridge::toCustomString(selectedId));
        item.setQuantity(1);
        item.setDueDate(bridge::toCoreDate(dueDate));
        item.setReturnDate(pbl2::core::Date());  // invalid -> chưa trả
        item.setStatus(bridge::toCustomString(QStringLiteral("BORROWED")));
        item.setFine(0);
        item.setExtensionCount(0);

        if (editRow >= 0 && static_cast<size_t>(editRow) < loanItems.size()) {
            loanItems[static_cast<size_t>(editRow)] = item;
        } else {
            loanItems.append(item);
        }

        refreshItemsList();
        return true;
    }

    void LoanDialog::refreshItemsList() {
        if (!itemsList) return;
        itemsList->clear();
        for (const auto &item : loanItems) {
            const QString bookId = bridge::toQString(item.getBookId());
            QString title;
            for (const auto &book : books) {
                if (bridge::toQString(book.getId()).compare(bookId, Qt::CaseInsensitive) == 0) {
                    title = bridge::toQString(book.getTitle());
                    break;
                }
            }
        const QString summary = tr("%1 (%2) - Hạn: %3")
                                        .arg(title.isEmpty() ? tr("Sách %1").arg(bookId) : title,
                                             bookId)
                                        .arg(item.getDueDate().isValid()
                                                 ? bridge::toQDate(item.getDueDate()).toString(QStringLiteral("dd/MM/yyyy"))
                                                 : tr("Không rõ"));
            itemsList->addItem(summary);
        }
        editItemButton->setEnabled(itemsList->count() > 0);
        removeItemButton->setEnabled(itemsList->count() > 0);
    }

    Loan LoanDialog::loan() const {
        Loan loan;
        loan.setLoanId(bridge::toCustomString(loanIdEdit->text().trimmed()));
        loan.setReaderId(bridge::toCustomString(readerCombo->currentData().toString()));
        loan.setBorrowDate(bridge::toCoreDate(borrowDateEdit->date()));
        loan.setStatus(bridge::toCustomString(QStringLiteral("BORROWED")));
        loan.setFine(0);
        loan.setStaffUsername(bridge::toCustomString(staffEdit->text().trimmed()));
        loan.setItems(loanItems);
        return loan;
    }

    bool LoanDialog::validateInputs() const {
        if (loanIdEdit->text().trimmed().isEmpty()) {
            return false;
        }
        if (readerCombo->currentIndex() < 0) {
            return false;
        }
        if (loanItems.isEmpty()) {
            return false;
        }
        const QDate borrowDate = borrowDateEdit ? borrowDateEdit->date() : bridge::currentDate();
        for (const auto &item : loanItems) {
            if (!item.getDueDate().isValid()) return false;
            if (bridge::toQDate(item.getDueDate()) < borrowDate) return false;
        }
        return true;
    }

    void LoanDialog::showError(const QString &message) const {
        errorLabel->setText(message);
        errorLabel->setVisible(true);
    }

    void LoanDialog::accept() {
        errorLabel->setVisible(false);
        if (!validateInputs()) {
            showError(tr("Vui lòng kiểm tra các trường bắt buộc."));
            return;
        }
        QDialog::accept();
    }

}

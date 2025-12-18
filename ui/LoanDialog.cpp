#include "LoanDialog.h"

#include "QtBridge.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QCompleter>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVariant>
#include <QVBoxLayout>

using namespace std;

namespace {

using pbl2::model::Book;
using pbl2::model::Loan;
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

        loanIdEdit = new QLineEdit(this);
        readerCombo = new QComboBox(this);
        bookCombo = new QComboBox(this);
        staffEdit = new QLineEdit(this);
        staffEdit->setReadOnly(true);

        for (const auto &reader : readers) {
            readerCombo->addItem(displayReader(reader), QVariant(bridge::toQString(reader.getId())));
        }
        for (const auto &book : books) {
            const QString bookId = bridge::toQString(book.getId());
            bookCombo->addItem(displayBook(book), QVariant(bookId));
        }

        setupSearchableCombo(readerCombo, tr("Tìm bạn đọc"));
        setupSearchableCombo(bookCombo, tr("Tìm sách"));

        readerCombo->setCurrentIndex(-1);
        bookCombo->setCurrentIndex(-1);
        readerCombo->clearEditText();
        bookCombo->clearEditText();

        staffEdit->setText(staffUsername);

        borrowDateEdit = new QDateEdit(this);
        borrowDateEdit->setCalendarPopup(true);
        borrowDateEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
        borrowDateEdit->setDate(bridge::currentDate());

        dueDateEdit = new QDateEdit(this);
        dueDateEdit->setCalendarPopup(true);
        dueDateEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
        dueDateEdit->setDate(bridge::currentDate().addDays(maxBorrowDays > 0 ? maxBorrowDays : 14));

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
        form->addRow(tr("Sách"), bookCombo);
        form->addRow(tr("Nhân viên lập phiếu"), staffEdit);
        form->addRow(tr("Ngày mượn"), borrowDateEdit);
        form->addRow(tr("Ngày trả hạn"), dueDateEdit);
        formGroup->setLayout(form);

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
        setMinimumSize(600, 500);
    }

    void LoanDialog::presetLoanId(const QString &loanId, bool lockField) {
        forceIdReadOnly = lockField;
        loanIdEdit->setText(loanId.trimmed());
        loanIdEdit->setReadOnly(editingMode || forceIdReadOnly);
    }

    void LoanDialog::presetBook(const QString &bookId) {
        if (!bookCombo) return;
        const QString trimmedId = bookId.trimmed();
        if (trimmedId.isEmpty()) return;

        for (int i = 0; i < bookCombo->count(); ++i) {
            const QString optionId = bookCombo->itemData(i).toString();
            if (optionId.compare(trimmedId, Qt::CaseInsensitive) == 0) {
                bookCombo->setCurrentIndex(i);
                readerCombo->setFocus();
                break;
            }
        }
    }

    Loan LoanDialog::loan() const {
        Loan loan;
        loan.setLoanId(bridge::toCustomString(loanIdEdit->text().trimmed()));
        loan.setReaderId(bridge::toCustomString(readerCombo->currentData().toString()));
        loan.setBookId(bridge::toCustomString(bookCombo->currentData().toString()));
        loan.setBorrowDate(bridge::toCoreDate(borrowDateEdit->date()));
        loan.setDueDate(bridge::toCoreDate(dueDateEdit->date()));
        loan.setStatus(bridge::toCustomString(QStringLiteral("BORROWED")));
        loan.setFine(0);
        return loan;
    }

    bool LoanDialog::validateInputs() const {
        if (loanIdEdit->text().trimmed().isEmpty()) {
            return false;
        }
        if (readerCombo->currentIndex() < 0 || bookCombo->currentIndex() < 0) {
            return false;
        }
        if (dueDateEdit->date() < borrowDateEdit->date()) {
            return false;
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

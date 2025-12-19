#pragma once

#include <QDialog>
#include <QString>
#include "../core/DynamicArray.h"

#include "model/Book.h"
#include "model/Loan.h"
#include "model/Reader.h"

class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

namespace pbl2::ui{

class LoanDialog final : public QDialog {
    Q_OBJECT

public:
    LoanDialog(const core::DynamicArray<model::Reader> &readers,
               const core::DynamicArray<model::Book> &books,
               int maxBorrowDays,
               const QString &staffUsername,
               QWidget *parent = nullptr);

    void presetLoanId(const QString &loanId, bool lockField = true);
    void presetInitialBook(const QString &bookId);
    [[nodiscard]] model::Loan loan() const;

protected:
    void accept() override;

private:
    bool showItemEditor(int editRow = -1);
    void refreshItemsList();
    [[nodiscard]] bool validateInputs() const;
    void showError(const QString &message) const;

    core::DynamicArray<model::Reader> readers;
    core::DynamicArray<model::Book> books;
    core::DynamicArray<model::LoanItem> loanItems;
    int maxBorrowDays{14};
    QLineEdit *loanIdEdit{nullptr};
    QComboBox *readerCombo{nullptr};
    QListWidget *itemsList{nullptr};
    QPushButton *addItemButton{nullptr};
    QPushButton *editItemButton{nullptr};
    QPushButton *removeItemButton{nullptr};
    QLineEdit *staffEdit{nullptr};
    QDateEdit *borrowDateEdit{nullptr};
    QLabel *errorLabel{nullptr};
    QDialogButtonBox *buttonBox{nullptr};
    bool editingMode{false};
    bool forceIdReadOnly{false};
};

}  // namespace ui

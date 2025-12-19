#pragma once
#include "core/Date.h"
#include "../../core/DynamicArray.h"
#include "../../core/CustomString.h"

namespace pbl2::model {

struct LoanItem {
    [[nodiscard]] const core::CustomString &getBookId() const;
    void setBookId(const core::CustomString &value);

    [[nodiscard]] int getQuantity() const;
    void setQuantity(int value);

    [[nodiscard]] const core::Date &getDueDate() const;
    void setDueDate(const core::Date &value);

    [[nodiscard]] const core::Date &getReturnDate() const;
    void setReturnDate(const core::Date &value);

    [[nodiscard]] const core::CustomString &getStatus() const;
    void setStatus(const core::CustomString &value);

    [[nodiscard]] int getFine() const;
    void setFine(int value);

    [[nodiscard]] int getExtensionCount() const;
    void setExtensionCount(int value);

private:
    core::CustomString bookId;
    int quantity{1};
    core::Date dueDate;
    core::Date returnDate;
    core::CustomString status;
    int fine{0};
    int extensionCount{0};
};

struct Loan {
    [[nodiscard]] const core::CustomString &getLoanId() const;
    void setLoanId(const core::CustomString &value);

    [[nodiscard]] const core::CustomString &getReaderId() const;
    void setReaderId(const core::CustomString &value);

    [[nodiscard]] const core::CustomString &getBookId() const;
    void setBookId(const core::CustomString &value);

    [[nodiscard]] const core::Date &getBorrowDate() const;
    void setBorrowDate(const core::Date &value);

    [[nodiscard]] const core::Date &getDueDate() const;
    void setDueDate(const core::Date &value);

    [[nodiscard]] const core::Date &getReturnDate() const;
    void setReturnDate(const core::Date &value);

    [[nodiscard]] const core::CustomString &getStatus() const;
    void setStatus(const core::CustomString &value);

    [[nodiscard]] int getFine() const;
    void setFine(int value);

    [[nodiscard]] int getExtensionCount() const;

    [[nodiscard]] const core::CustomString &getStaffUsername() const;
    void setStaffUsername(const core::CustomString &value);

    [[nodiscard]] const core::DynamicArray<LoanItem> &getItems() const;
    void setItems(const core::DynamicArray<LoanItem> &value);
    void addItem(const LoanItem &item);

private:
    core::CustomString loanId;
    core::CustomString readerId;
    core::CustomString bookId;
    core::Date borrowDate;
    core::Date dueDate;
    core::Date returnDate;
    core::CustomString status;
    int fine{0};
    core::CustomString staffUsername;
    core::DynamicArray<LoanItem> items;
};

}  // namespace model

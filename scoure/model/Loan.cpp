#include "model/Loan.h"

namespace pbl2::model {

const core::CustomString &LoanItem::getBookId() const {
    return bookId;
}

void LoanItem::setBookId(const core::CustomString &value) {
    bookId = value;
}

int LoanItem::getQuantity() const {
    return quantity;
}

void LoanItem::setQuantity(const int value) {
    quantity = value;
}

const core::Date &LoanItem::getDueDate() const {
    return dueDate;
}

void LoanItem::setDueDate(const core::Date &value) {
    dueDate = value;
}

const core::Date &LoanItem::getReturnDate() const {
    return returnDate;
}

void LoanItem::setReturnDate(const core::Date &value) {
    returnDate = value;
}

const core::CustomString &LoanItem::getStatus() const {
    return status;
}

void LoanItem::setStatus(const core::CustomString &value) {
    status = value;
}

int LoanItem::getFine() const {
    return fine;
}

void LoanItem::setFine(const int value) {
    fine = value;
}

int LoanItem::getExtensionCount() const {
    return extensionCount;
}

void LoanItem::setExtensionCount(const int value) {
    extensionCount = value;
}

const core::CustomString &Loan::getLoanId() const {
    return loanId;
}

void Loan::setLoanId(const core::CustomString &value) {
    loanId = value;
}

const core::CustomString &Loan::getReaderId() const {
    return readerId;
}

void Loan::setReaderId(const core::CustomString &value) {
    readerId = value;
}

const core::CustomString &Loan::getBookId() const {
    return bookId;
}

void Loan::setBookId(const core::CustomString &value) {
    bookId = value;
}

const core::Date &Loan::getBorrowDate() const {
    return borrowDate;
}

void Loan::setBorrowDate(const core::Date &value) {
    borrowDate = value;
}

const core::Date &Loan::getDueDate() const {
    return dueDate;
}

void Loan::setDueDate(const core::Date &value) {
    dueDate = value;
}

const core::Date &Loan::getReturnDate() const {
    return returnDate;
}

void Loan::setReturnDate(const core::Date &value) {
    returnDate = value;
}

const core::CustomString &Loan::getStatus() const {
    return status;
}

void Loan::setStatus(const core::CustomString &value) {
    status = value;
}

int Loan::getFine() const {
    return fine;
}

void Loan::setFine(const int value) {
    fine = value;
}

int Loan::getExtensionCount() const {
    int total = 0;
    for (const auto &item : items) {
        total += item.getExtensionCount();
    }
    return total;
}

const core::CustomString &Loan::getStaffUsername() const {
    return staffUsername;
}

void Loan::setStaffUsername(const core::CustomString &value) {
    staffUsername = value;
}

const core::DynamicArray<LoanItem> &Loan::getItems() const {
    return items;
}

void Loan::setItems(const core::DynamicArray<LoanItem> &value) {
    items = value;
    if (!items.isEmpty()) {
        bookId = items[0].getBookId();
        dueDate = items[0].getDueDate();
        returnDate = items[0].getReturnDate();
    }
}

void Loan::addItem(const LoanItem &item) {
    items.append(item);
    if (items.size() == 1) {
        bookId = item.getBookId();
        dueDate = item.getDueDate();
        returnDate = item.getReturnDate();
    }
}

}  // namespace pbl2::model

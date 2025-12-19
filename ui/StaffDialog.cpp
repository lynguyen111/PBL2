#include "StaffDialog.h"

#include "QtBridge.h"
#include "service/StaffService.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

using namespace std;  // project-wide request

namespace {
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

StaffDialog::StaffDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Thông tin nhân viên"));
    setModal(true);
    setWindowIcon(QIcon(":/ui/resources/icons/staff.png"));
    const QFont font("Segoe UI", 11);
    setFont(font);
    setStyleSheet("QDialog { background: #f8fafc; border-radius: 12px; } QGroupBox { font-weight: bold; } QLineEdit, QComboBox, QSpinBox, QDateEdit, QPlainTextEdit { min-height: 32px; font-size: 11pt; } QDialogButtonBox QPushButton { min-width: 120px; min-height: 40px; font-size: 11pt; font-weight: 600; } QLabel { font-size: 11pt; } ");

    idEdit = new QLineEdit(this);
        idEdit->setValidator(new QIntValidator(0, 2147483647, this)); // Chỉ cho phép nhập số
    fullNameEdit = new QLineEdit(this);
    genderCombo = new QComboBox(this);
    genderCombo->setEditable(true);
    genderCombo->addItems({tr("Chọn"), tr("Nam"), tr("Nữ"), tr("Khác")});
    genderCombo->setCurrentIndex(-1);
    genderCombo->setEditText(QString());
    addressEdit = new QLineEdit(this);
    phoneEdit = new QLineEdit(this);
        phoneEdit->setValidator(new QIntValidator(0, 2147483647, this)); // Chỉ cho phép nhập số
    emailEdit = new QLineEdit(this);
    positionCombo = new QComboBox(this);
    positionCombo->setEditable(true);
    positionCombo->addItems({tr("Chọn"), tr("Thủ thư"), tr("Quản lý"), tr("Nhân viên"), tr("Trợ lý"), tr("Khác")});
    positionCombo->setCurrentIndex(-1);
    positionCombo->setEditText(QString());
    notesEdit = new QLineEdit(this);
    dobEdit = new QDateEdit(this);
    dobEdit->setCalendarPopup(true);
    dobEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    dobEdit->setDate(bridge::currentDate());
    hireDateEdit = new QDateEdit(this);
    hireDateEdit->setCalendarPopup(true);
    hireDateEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    hireDateEdit->setDate(bridge::currentDate());

    activeCheck = new QCheckBox(tr("Đang hoạt động"), this);
    activeCheck->setChecked(true);
    connect(activeCheck, &QCheckBox::toggled, this, [this](bool checked) {
        activeFlag = checked;
    });

    errorLabel = new QLabel(this);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setVisible(false);
    

    auto *formGroup = new QGroupBox(tr("Thông tin nhân viên"), this);
    auto *form = new QFormLayout;
    form->setContentsMargins(12, 12, 12, 12);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->addRow(tr("Mã nhân viên"), idEdit);
    form->addRow(tr("Họ tên <span style=\"color:#dc2626;\">*</span>"), fullNameEdit);
    form->addRow(tr("Giới tính <span style=\"color:#dc2626;\">*</span>"), genderCombo);
    form->addRow(tr("Địa chỉ <span style=\"color:#dc2626;\">*</span>"), addressEdit);
    form->addRow(tr("Số điện thoại <span style=\"color:#dc2626;\">*</span>"), phoneEdit);
    form->addRow(tr("Email <span style=\"color:#dc2626;\">*</span>"), emailEdit);
    form->addRow(tr("Chức vụ <span style=\"color:#dc2626;\">*</span>"), positionCombo);
    form->addRow(tr("Ghi chú"), notesEdit);
    form->addRow(tr("Ngày sinh"), dobEdit);
    form->addRow(tr("Ngày vào làm"), hireDateEdit);
    form->addRow(tr("Trạng thái"), activeCheck);
    formGroup->setLayout(form);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->setCenterButtons(true);
    if (auto *okBtn = buttonBox->button(QDialogButtonBox::Ok)) {
        okBtn->setMinimumSize(120, 40);
        okBtn->setDefault(true);
        okBtn->setStyleSheet(primaryActionStyle());
    }
    if (auto *cancelBtn = buttonBox->button(QDialogButtonBox::Cancel)) {
        cancelBtn->setMinimumSize(120, 40);
        cancelBtn->setStyleSheet(dangerActionStyle());
    }
    connect(buttonBox, &QDialogButtonBox::accepted, this, &StaffDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &StaffDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addWidget(formGroup);
    layout->addWidget(errorLabel);
    layout->addWidget(buttonBox, 0, Qt::AlignRight);
    setMinimumSize(640, 600);
}

void StaffDialog::setStaff(const model::Staff &staff, bool editing) {
    editingMode = editing;
    idEdit->setText(bridge::toQString(staff.getId()));
    idEdit->setReadOnly(editing || forceIdReadOnly);
    idEdit->setEnabled(!editing && !forceIdReadOnly);
    fullNameEdit->setText(bridge::toQString(staff.getFullName()));
    if (const QString genderText = bridge::toQString(staff.getGender()).trimmed(); !genderText.isEmpty()) {
        genderCombo->setCurrentText(genderText);
    } else {
        genderCombo->setCurrentIndex(-1);
        genderCombo->setEditText(QString());
    }
    addressEdit->setText(bridge::toQString(staff.getAddress()));
    phoneEdit->setText(bridge::toQString(staff.getPhone()));
    emailEdit->setText(bridge::toQString(staff.getEmail()));
    if (const QString positionText = bridge::toQString(staff.getPosition()).trimmed(); !positionText.isEmpty()) {
        if (positionCombo->findText(positionText, Qt::MatchFixedString | Qt::MatchCaseSensitive) == -1) {
            positionCombo->addItem(positionText);
        }
        positionCombo->setCurrentText(positionText);
    } else {
        positionCombo->setCurrentIndex(-1);
        positionCombo->setEditText(QString());
    }
    notesEdit->setText(bridge::toQString(staff.getNotes()));
    if (staff.getDob().isValid()) {
        dobEdit->setDate(bridge::toQDate(staff.getDob()));
    } else {
        dobEdit->setDate(bridge::currentDate());
    }
    if (staff.getHireDate().isValid()) {
        hireDateEdit->setDate(bridge::toQDate(staff.getHireDate()));
    } else {
        hireDateEdit->setDate(bridge::currentDate());
    }
    activeFlag = staff.isActive();
    activeCheck->setChecked(activeFlag);

    if (editing) {
        idEdit->setReadOnly(true);
        idEdit->setEnabled(false);
        fullNameEdit->setReadOnly(true);
    } else {
        idEdit->setEnabled(!forceIdReadOnly);
        fullNameEdit->setReadOnly(false);
    }
}

void StaffDialog::presetId(const QString &id, bool lockField) {
    forceIdReadOnly = lockField;
    idEdit->setText(id.trimmed());
    idEdit->setReadOnly(editingMode || forceIdReadOnly);
}

model::Staff StaffDialog::staff() const {
    model::Staff s;
    s.setId(bridge::toCustomString(idEdit->text().trimmed()));
    s.setFullName(bridge::toCustomString(fullNameEdit->text().trimmed()));
    s.setGender(bridge::toCustomString(genderCombo->currentText().trimmed()));
    s.setAddress(bridge::toCustomString(addressEdit->text().trimmed()));
    s.setPhone(bridge::toCustomString(phoneEdit->text().trimmed()));
    s.setEmail(bridge::toCustomString(emailEdit->text().trimmed()));
    s.setPosition(bridge::toCustomString(positionCombo->currentText().trimmed()));
    s.setNotes(bridge::toCustomString(notesEdit->text().trimmed()));
    s.setDob(bridge::toCoreDate(dobEdit->date()));
    s.setHireDate(bridge::toCoreDate(hireDateEdit->date()));
    s.setActive(activeFlag);
    return s;
}

bool StaffDialog::validateInputs() const {
    if (idEdit->text().trimmed().isEmpty()) {
        showError(tr("Mã nhân viên không được để trống"));
        return false;
    }
    if (fullNameEdit->text().trimmed().isEmpty()) {
        showError(tr("Họ tên không được để trống."));
        return false;
    }
    const QString genderText = genderCombo->currentText().trimmed();
    if (genderText.isEmpty() || genderText.compare(tr("Chọn"), Qt::CaseInsensitive) == 0) {
        showError(tr("Giới tính không được để trống."));
        return false;
    }
    if (addressEdit->text().trimmed().isEmpty()) {
        showError(tr("Địa chỉ không được để trống."));
        return false;
    }
    if (phoneEdit->text().trimmed().isEmpty()) {
        showError(tr("Số điện thoại không được để trống."));
        return false;
    }
    if (emailEdit->text().trimmed().isEmpty()) {
        showError(tr("Email không được để trống."));
        return false;
    }
    const QString positionText = positionCombo->currentText().trimmed();
    if (positionText.isEmpty() || positionText.compare(tr("Chọn"), Qt::CaseInsensitive) == 0) {
        showError(tr("Chức vụ không được để trống."));
        return false;
    }
    return true;
}

void StaffDialog::showError(const QString &message) const {
    errorLabel->setText(message);
    errorLabel->setVisible(true);
}

void StaffDialog::accept() {
    errorLabel->setVisible(false);
    if (!validateInputs()) return;

    // Chỉ kiểm tra trùng khi thêm mới
    if (!editingMode) {
        extern pbl2::service::StaffService *staffService;
        const auto phone = phoneEdit->text().trimmed();
        if (staffService && staffService->isDuplicatePhone(bridge::toCustomString(phone))) {
            showError(tr("Số điện thoại đã tồn tại trong hệ thống."));
            return;
        }
    }
    QDialog::accept();
}

}  // namespace ui

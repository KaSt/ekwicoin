// Copyright (c) 2011-2014 The Ekwicoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ekwicoinamountfield.h"

#include "ekwicoinunits.h"
#include "guiconstants.h"
#include "qvaluecombobox.h"

#include <QApplication>
#include <QAbstractSpinBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>

/** QSpinBox that uses fixed-point numbers internally and uses our own
 * formatting/parsing functions.
 */
class AmountSpinBox: public QAbstractSpinBox
{
    Q_OBJECT
public:
    explicit AmountSpinBox(QWidget *parent):
        QAbstractSpinBox(parent),
        currentUnit(EkwicoinUnits::EKW),
        singleStep(100000) // satoshis
    {
        setAlignment(Qt::AlignRight);

        connect(lineEdit(), SIGNAL(textEdited(QString)), this, SIGNAL(valueChanged()));
    }

    QValidator::State validate(QString &text, int &pos) const
    {
        if(text.isEmpty())
            return QValidator::Intermediate;
        bool valid = false;
        parse(text, &valid);
        /* Make sure we return Intermediate so that fixup() is called on defocus */
        return valid ? QValidator::Intermediate : QValidator::Invalid;
    }

    void fixup(QString &input) const
    {
        bool valid = false;
        qint64 val = parse(input, &valid);
        if(valid)
        {
            input = EkwicoinUnits::format(currentUnit, val, false, EkwicoinUnits::separatorAlways);
            lineEdit()->setText(input);
        }
    }

    qint64 value(bool *valid_out=0) const
    {
        return parse(text(), valid_out);
    }

    void setValue(qint64 value)
    {
        lineEdit()->setText(EkwicoinUnits::format(currentUnit, value, false, EkwicoinUnits::separatorAlways));
        emit valueChanged();
    }

    void stepBy(int steps)
    {
        bool valid = false;
        qint64 val = value(&valid);
        val = val + steps * singleStep;
        val = qMin(qMax(val, Q_INT64_C(0)), EkwicoinUnits::maxMoney());
        setValue(val);
    }

    StepEnabled stepEnabled() const
    {
        StepEnabled rv = 0;
        if(text().isEmpty()) // Allow step-up with empty field
            return StepUpEnabled;
        bool valid = false;
        qint64 val = value(&valid);
        if(valid)
        {
            if(val > 0)
                rv |= StepDownEnabled;
            if(val < EkwicoinUnits::maxMoney())
                rv |= StepUpEnabled;
        }
        return rv;
    }

    void setDisplayUnit(int unit)
    {
        bool valid = false;
        qint64 val = value(&valid);

        currentUnit = unit;

        if(valid)
            setValue(val);
        else
            clear();
    }

    void setSingleStep(qint64 step)
    {
        singleStep = step;
    }

    QSize minimumSizeHint() const
    {
        if(cachedMinimumSizeHint.isEmpty())
        {
            ensurePolished();

            const QFontMetrics fm(fontMetrics());
            int h = lineEdit()->minimumSizeHint().height();
            int w = fm.width(EkwicoinUnits::format(EkwicoinUnits::EKW, EkwicoinUnits::maxMoney(), false, EkwicoinUnits::separatorAlways));
            w += 2; // cursor blinking space

            QStyleOptionSpinBox opt;
            initStyleOption(&opt);
            QSize hint(w, h);
            QSize extra(35, 6);
            opt.rect.setSize(hint + extra);
            extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                    QStyle::SC_SpinBoxEditField, this).size();
            // get closer to final result by repeating the calculation
            opt.rect.setSize(hint + extra);
            extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                    QStyle::SC_SpinBoxEditField, this).size();
            hint += extra;

            opt.rect = rect();

            cachedMinimumSizeHint = style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
                                    .expandedTo(QApplication::globalStrut());
        }
        return cachedMinimumSizeHint;
    }
private:
    int currentUnit;
    qint64 singleStep;
    mutable QSize cachedMinimumSizeHint;

    /**
     * Parse a string into a number of base monetary units and
     * return validity.
     * @note Must return 0 if !valid.
     */
    qint64 parse(const QString &text, bool *valid_out=0) const
    {
        qint64 val = 0;
        bool valid = EkwicoinUnits::parse(currentUnit, text, &val);
        if(valid)
        {
            if(val < 0 || val > EkwicoinUnits::maxMoney())
                valid = false;
        }
        if(valid_out)
            *valid_out = valid;
        return valid ? val : 0;
    }

protected:
    bool event(QEvent *event)
    {
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Comma)
            {
                // Translate a comma into a period
                QKeyEvent periodKeyEvent(event->type(), Qt::Key_Period, keyEvent->modifiers(), ".", keyEvent->isAutoRepeat(), keyEvent->count());
                return QAbstractSpinBox::event(&periodKeyEvent);
            }
        }
        return QAbstractSpinBox::event(event);
    }

signals:
    void valueChanged();
};

#include "ekwicoinamountfield.moc"

EkwicoinAmountField::EkwicoinAmountField(QWidget *parent) :
    QWidget(parent),
    amount(0)
{
    amount = new AmountSpinBox(this);
    amount->setLocale(QLocale::c());
    amount->installEventFilter(this);
    amount->setMaximumWidth(170);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(amount);
    unit = new QValueComboBox(this);
    unit->setModel(new EkwicoinUnits(this));
    layout->addWidget(unit);
    layout->addStretch(1);
    layout->setContentsMargins(0,0,0,0);

    setLayout(layout);

    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(amount);

    // If one if the widgets changes, the combined content changes as well
    connect(amount, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    connect(unit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitChanged(int)));

    // Set default based on configuration
    unitChanged(unit->currentIndex());
}

void EkwicoinAmountField::clear()
{
    amount->clear();
    unit->setCurrentIndex(0);
}

bool EkwicoinAmountField::validate()
{
    bool valid = false;
    value(&valid);
    setValid(valid);
    return valid;
}

void EkwicoinAmountField::setValid(bool valid)
{
    if (valid)
        amount->setStyleSheet("");
    else
        amount->setStyleSheet(STYLE_INVALID);
}

bool EkwicoinAmountField::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
    {
        // Clear invalid flag on focus
        setValid(true);
    }
    return QWidget::eventFilter(object, event);
}

QWidget *EkwicoinAmountField::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, amount);
    QWidget::setTabOrder(amount, unit);
    return unit;
}

qint64 EkwicoinAmountField::value(bool *valid_out) const
{
    return amount->value(valid_out);
}

void EkwicoinAmountField::setValue(qint64 value)
{
    amount->setValue(value);
}

void EkwicoinAmountField::setReadOnly(bool fReadOnly)
{
    amount->setReadOnly(fReadOnly);
    unit->setEnabled(!fReadOnly);
}

void EkwicoinAmountField::unitChanged(int idx)
{
    // Use description tooltip for current unit for the combobox
    unit->setToolTip(unit->itemData(idx, Qt::ToolTipRole).toString());

    // Determine new unit ID
    int newUnit = unit->itemData(idx, EkwicoinUnits::UnitRole).toInt();

    amount->setDisplayUnit(newUnit);
}

void EkwicoinAmountField::setDisplayUnit(int newUnit)
{
    unit->setValue(newUnit);
}

void EkwicoinAmountField::setSingleStep(qint64 step)
{
    amount->setSingleStep(step);
}

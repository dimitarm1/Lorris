/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <qscriptsyntaxhighlighter_p.h>
#include <QPainter>
#include <QScrollBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include "scripteditor.h"
#include "../../../common.h"
#include "engines/scriptengine.h"
#include "engines/pythonhighlighter.h"


ScriptEditor::ScriptEditor(const QString& source, int type, const QString &widgetName) :
    QDialog(NULL, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);

    m_line_num = new LineNumber(this);
    ui->editLayout->insertWidget(0, m_line_num);

    m_highlighter = NULL;
    m_errors = 0;

#ifdef Q_OS_MAC
    ui->sourceEdit->setFont(Utils::getMonospaceFont(12));
#else
    ui->sourceEdit->setFont(Utils::getMonospaceFont());
#endif
    setWindowTitle(windowTitle() + widgetName);

    QScrollBar *bar = ui->sourceEdit->verticalScrollBar();
    connect(bar,            SIGNAL(rangeChanged(int,int)),     SLOT(rangeChanged(int,int)));
    connect(bar,            SIGNAL(valueChanged(int)),         SLOT(sliderMoved(int)));
    connect(ui->sourceEdit->document(), SIGNAL(contentsChange(int,int,int)),
                                        SLOT(contentsChange(int,int,int)));

    ui->sourceEdit->setPlainText(source);
    ui->sourceEdit->setTabStopWidth(ui->sourceEdit->fontMetrics().width(' ') * 4);

    m_changed = !source.isNull();

    ui->langBox->addItems(ScriptEngine::getEngineList());
    ui->langBox->setCurrentIndex(type);
    ui->errorEdit->hide();

    updateExampleList();
}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}

QString ScriptEditor::getSource()
{
    return ui->sourceEdit->toPlainText();
}

int ScriptEditor::getEngine()
{
    return ui->langBox->currentIndex();
}

void ScriptEditor::on_buttonBox_clicked(QAbstractButton *btn)
{
    switch(ui->buttonBox->buttonRole(btn))
    {
        case QDialogButtonBox::ApplyRole:  emit applySource(false); break;
        case QDialogButtonBox::AcceptRole: emit applySource(true);  break;
        default: break;
    }
}

void ScriptEditor::on_sourceEdit_textChanged()
{
    m_line_num->setLineNum(ui->sourceEdit->document()->lineCount());
}

void ScriptEditor::contentsChange(int /*position*/, int charsRemoved, int charsAdded)
{
    if(charsRemoved != charsAdded)
    {
        m_changed = true;
        ui->exampleBox->setCurrentIndex(0);
    }
}

void ScriptEditor::sliderMoved(int val)
{
    m_line_num->setScroll(val);
}

void ScriptEditor::rangeChanged(int, int)
{
    m_line_num->setScroll(ui->sourceEdit->verticalScrollBar()->value());
}

void ScriptEditor::on_loadBtn_clicked()
{
    static const QString filters[ENGINE_MAX] =
    {
        tr("JavaScript file (*.js);;Any file (*.*)"),
        tr("Python file (*.py);;Any file (*.*)"),
    };

    QString filename = QFileDialog::getOpenFileName(this, tr("Load file"),
                                                    sConfig.get(CFG_STRING_ANALYZER_JS),
                                                    filters[ui->langBox->currentIndex()]);
    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return Utils::ThrowException(tr("Failed to open \"%1!\"").arg(filename));

    ui->sourceEdit->clear();
    ui->sourceEdit->setPlainText(QString(file.readAll()));
    file.close();

    sConfig.set(CFG_STRING_ANALYZER_JS, filename);
}

void ScriptEditor::on_langBox_currentIndexChanged(int idx)
{
    if(!m_changed)
    {
        static const QString defaultCode[ENGINE_MAX] = {
            tr("// You can use clearTerm() and appendTerm(string) to set term content\n"
            "// You can use sendData(Array of ints) to send data to device. It expects array of uint8s\n\n"
            "// This function gets called on data received\n"
            "// it should return string, which is automatically appended to terminal\n"
            "function onDataChanged(data, dev, cmd, index) {\n"
            "\treturn \"\";\n"
            "}\n\n"
            "// This function is called on key press in terminal.\n"
            "// Param is string\n"
            "function onKeyPress(key) {\n"
            "\n"
            "}\n"),

            tr("# You can use terminal.clear() and terminal.appendText(string) to set term content\n"
            "# You can use lorris.sendData(QByteArray) to send data to device.\n"
            "\n"
            "# This function gets called on data received\n"
            "# it should return string, which is automatically appended to terminal\n"
            "def onDataChanged(data, dev, cmd, index):\n"
            "\treturn \"\";\n"
            "\n"
            "# This function is called on key press in terminal.\n"
            "# Param is string\n"
            "def onKeyPress(key):\n"
            "\treturn;\n")
        };

        ui->sourceEdit->setPlainText(defaultCode[idx]);
        m_changed = false;
    }

    delete m_highlighter;
    switch(idx)
    {
        case ENGINE_QTSCRIPT:
            m_highlighter = new QScriptSyntaxHighlighter(ui->sourceEdit->document());
            break;
        case ENGINE_PYTHON:
            m_highlighter = new PythonHighlighter(ui->sourceEdit->document());
            break;
        default:
            m_highlighter = NULL;
            break;
    }
    updateExampleList();
}

void ScriptEditor::on_errorBtn_toggled(bool checked)
{
    ui->errorEdit->setShown(checked);
}

void ScriptEditor::on_exampleBox_activated(int index)
{
    if(index == 0)
        return;

    if(m_changed)
    {
        QMessageBox box(QMessageBox::Question, tr("Load example"), tr("Script was changed, do you really want to load an example?"),
                       (QMessageBox::Yes | QMessageBox::No), this);

        if(box.exec() == QMessageBox::No)
        {
            ui->exampleBox->setCurrentIndex(0);
            return;
        }
    }

    QFile file(":/examples/" + ui->exampleBox->currentText());
    if(!file.open(QIODevice::ReadOnly))
        return;

    ui->sourceEdit->setPlainText(file.readAll());
    ui->exampleBox->setCurrentIndex(index);
    m_changed = false;
}

void ScriptEditor::addError(const QString& error)
{
    ui->errorEdit->insertPlainText(error);
    ++m_errors;
    ui->errorBtn->setText(tr("Show errors (%1)").arg(m_errors));
}

void ScriptEditor::clearErrors()
{
    ui->errorEdit->clear();
    m_errors = 0;
    ui->errorBtn->setText(tr("Show errors (%1)").arg(m_errors));
}

void ScriptEditor::updateExampleList()
{
    static const QStringList filters[ENGINE_MAX] =
    {
        (QStringList() << "*.js"),
        (QStringList() << "*.py")
    };

    while(ui->exampleBox->count() > 1)
        ui->exampleBox->removeItem(1);

    QDir dir(":/examples");
    ui->exampleBox->addItems(dir.entryList(filters[ui->langBox->currentIndex()], QDir::NoFilter, QDir::Name));
}

LineNumber::LineNumber(QWidget *parent) : QWidget(parent)
{
#ifdef Q_OS_MAC
    setFont(Utils::getMonospaceFont(12));
#else
    setFont(Utils::getMonospaceFont());
#endif
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setMinimumSize(5, 5);

    m_char_h = fontMetrics().height();
    m_line_num = 0;
    m_last_w = 0;
    m_scroll = 0;
}

void LineNumber::setLineNum(int lineNum)
{
    m_line_num = lineNum;
    update();
}

void LineNumber::setScroll(int line)
{
    m_scroll = line;
    update();
}

void LineNumber::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);

    int h = 5;
    for(int line = m_scroll; h+m_char_h < height() && line < m_line_num; ++line)
    {
        QString text;
        text.setNum(line+1);

        int text_w = fontMetrics().width(text);
        if(m_last_w < text_w)
        {
            m_last_w = text_w;
            setFixedWidth(text_w);
        }

        painter.drawText(0, h, m_last_w, m_char_h, Qt::AlignRight, text);
        h += m_char_h;
    }
}

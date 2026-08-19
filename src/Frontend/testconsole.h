#ifndef TESTCONSOLE_H
#define TESTCONSOLE_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QLineEdit>

class TestConsole : public QDialog {
    Q_OBJECT

public:
    TestConsole(QWidget *parent = nullptr);

private slots:
    void onReturnPressed();

private:
    void processCommand(const QString &cmd);
    
    QPlainTextEdit *output;
    QLineEdit *input;
};

#endif // TESTCONSOLE_H
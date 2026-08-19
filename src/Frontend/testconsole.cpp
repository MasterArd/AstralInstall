#include "testconsole.h"
#include <QVBoxLayout>
#include <QFont>

TestConsole::TestConsole(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Test Console");
    setGeometry(100, 100, 600, 400);
    
    // Output area
    output = new QPlainTextEdit();
    output->setReadOnly(true);
    QFont font("Courier", 10);
    output->setFont(font);
    output->appendPlainText("=== Test Console ===");
    output->appendPlainText("Type 'help' for commands\n");
    
    // Input field
    input = new QLineEdit();
    input->setPlaceholderText("Enter command...");
    input->setFont(font);
    
    connect(input, &QLineEdit::returnPressed, this, &TestConsole::onReturnPressed);
    
    // Layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(output, 1);
    layout->addWidget(input);
    setLayout(layout);
    
    // Focus on input
    input->setFocus();
}

void TestConsole::onReturnPressed() {
    QString cmd = input->text();
    if (cmd.isEmpty()) return;
    
    output->appendPlainText("> " + cmd);
    processCommand(cmd);
    input->clear();
}

void TestConsole::processCommand(const QString &cmd) {
    QString response;
    
    if (cmd == "help") {
        response = "Commands:\n  test1\n  test2 <param>\n  clear\n  exit";
    }
    else if (cmd == "test1") {
        response = "Test 1 result: OK";
        // ADD YOUR BACKEND FUNCTION HERE
    }
    else if (cmd.startsWith("test2")) {
        response = "Test 2 result: " + cmd.mid(6);
        // ADD YOUR BACKEND FUNCTION HERE
    }
    else if (cmd == "clear") {
        output->clear();
        return;
    }
    else if (cmd == "exit") {
        close();
        return;
    }
    else {
        response = "Unknown command. Type 'help'";
    }
    
    output->appendPlainText(response + "\n");
}
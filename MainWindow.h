#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include "logic/quiz.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(std::shared_ptr<QuestionDatabase> database, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTopicChanged(int index);
    void onAddTopic();
    void onRemoveTopic();
    void onAddQuestion();
    void onRemoveQuestion();
    void onGenerateQuiz();
    void onSaveDatabase();
    void onEditQuestion();
    void onImport();
    void onQuestionSelectionChanged();
    void onLoadDatabase();
    void onAbout();

private:
    void setupUi();
    void setupMenus();
    void updateTopicsCombo();
    void updateQuestionsTable();
    void displayQuestion(const std::shared_ptr<Question>& question);
    void showError(const QString& message);
    void showInfo(const QString& message);
    bool confirm(const QString& message);

    std::shared_ptr<QuestionDatabase> db;
    
    // UI Elements
    QComboBox* topicsCombo;
    QTableWidget* questionsTable;
    QPushButton* addTopicBtn;
    QPushButton* removeTopicBtn;
    QPushButton* addQuestionBtn;
    QPushButton* editQuestionBtn;
    QPushButton* batchImportBtn;
    QPushButton* removeQuestionBtn;
    QPushButton* generateQuizBtn;
    QStatusBar* statusBar;
};
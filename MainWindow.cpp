#include "MainWindow.h"
#include <QDate>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QListWidget>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QStringList>
#include <random>

MainWindow::MainWindow(std::shared_ptr<QuestionDatabase> database, QWidget *parent)
    : QMainWindow(parent), db(database)
{
    setWindowTitle("MadExam - Система создания тестов");
    resize(800, 600);
    
    setupUi();
    setupMenus();
    updateTopicsCombo();
    
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Готово");
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    QHBoxLayout* topicLayout = new QHBoxLayout();
    QLabel* topicLabel = new QLabel("Тема:", this);
    topicsCombo = new QComboBox(this);
    topicsCombo->setMinimumWidth(300);
    connect(topicsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTopicChanged);
    
    addTopicBtn = new QPushButton("Добавить тему", this);
    connect(addTopicBtn, &QPushButton::clicked, this, &MainWindow::onAddTopic);
    
    removeTopicBtn = new QPushButton("Удалить тему", this);
    connect(removeTopicBtn, &QPushButton::clicked, this, &MainWindow::onRemoveTopic);
    
    topicLayout->addWidget(topicLabel);
    topicLayout->addWidget(topicsCombo);
    topicLayout->addWidget(addTopicBtn);
    topicLayout->addWidget(removeTopicBtn);
    mainLayout->addLayout(topicLayout);
    
    QGroupBox* questionsGroup = new QGroupBox("Вопросы", this);
    QVBoxLayout* questionsLayout = new QVBoxLayout(questionsGroup);
    
    questionsTable = new QTableWidget(this);
    questionsTable->setColumnCount(3);
    questionsTable->setHorizontalHeaderLabels({"Вопрос", "Тип", "Опции"});
    questionsTable->horizontalHeader()->setStretchLastSection(true);
    questionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    questionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(questionsTable, &QTableWidget::itemDoubleClicked, this, &MainWindow::onEditQuestion);
    connect(questionsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onQuestionSelectionChanged);
    questionsLayout->addWidget(questionsTable);
    
    QHBoxLayout* questionBtnLayout = new QHBoxLayout();
    addQuestionBtn = new QPushButton("Добавить вопрос", this);
    connect(addQuestionBtn, &QPushButton::clicked, this, &MainWindow::onAddQuestion);

    batchImportBtn = new QPushButton("Импортировать вопросы", this);
    connect(batchImportBtn, &QPushButton::clicked, this, &MainWindow::onImport);
    batchImportBtn->setEnabled(false);
    
    editQuestionBtn = new QPushButton("Редактировать", this);
    connect(editQuestionBtn, &QPushButton::clicked, this, &MainWindow::onEditQuestion);
    editQuestionBtn->setEnabled(false); // Disabled until a question is selected
    
    removeQuestionBtn = new QPushButton("Удалить вопрос", this);
    connect(removeQuestionBtn, &QPushButton::clicked, this, &MainWindow::onRemoveQuestion);
    
    generateQuizBtn = new QPushButton("Создать тест", this);
    connect(generateQuizBtn, &QPushButton::clicked, this, &MainWindow::onGenerateQuiz);
    
    questionBtnLayout->addWidget(addQuestionBtn);
    questionBtnLayout->addWidget(editQuestionBtn);
    questionBtnLayout->addWidget(batchImportBtn);
    questionBtnLayout->addWidget(removeQuestionBtn);
    questionBtnLayout->addWidget(generateQuizBtn);
    questionsLayout->addLayout(questionBtnLayout);
    
    mainLayout->addWidget(questionsGroup);
}

void MainWindow::onImport()
{
    QDialog importDialog(this);
    importDialog.setWindowTitle("Импорт вопросов");
    importDialog.setMinimumWidth(600);
    importDialog.setMinimumHeight(400);

    QVBoxLayout* importLayout = new QVBoxLayout(&importDialog);
    QLabel* instructionLabel = new QLabel("Введите несколько вопросов с файла. Каждый вопрос должен начинаться на цифру с точкой (1., 2., т.д.). После вопроса должны следовать варианты ответа.");
    instructionLabel->setWordWrap(true);
    importLayout->addWidget(instructionLabel);

    QPlainTextEdit* textEdit = new QPlainTextEdit(&importDialog);
    importLayout->addWidget(textEdit);

    QDialogButtonBox* importButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &importDialog);
    connect(importButtonBox, &QDialogButtonBox::accepted, &importDialog, &QDialog::accept);
    connect(importButtonBox, &QDialogButtonBox::rejected, &importDialog, &QDialog::reject);
    importLayout->addWidget(importButtonBox);

    if (importDialog.exec() == QDialog::Accepted) {
            QString text = textEdit->toPlainText().trimmed();
            if (text.isEmpty()) {
                return;
            }

            QRegularExpression questionBlockRegex(R"((\d+)\.\s*(.+?)(?=(?:\n\d+\.|\z)))", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpression optionRegex(R"(^\s*([A-ZА-Яa-zа-я0-9])\)\s*(.*)$)");

            QRegularExpressionMatchIterator it = questionBlockRegex.globalMatch(text);

            int addedCount = 0;
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QString rawBlock = match.captured(2).trimmed();
                QStringList lines = rawBlock.split('\n');

                QString questionTextContent;
                QStringList options;
                int firstOptionIndex = -1;

                // Find first line that matches option pattern
                for (int i = 0; i < lines.size(); ++i) {
                    if (optionRegex.match(lines[i]).hasMatch()) {
                        firstOptionIndex = i;
                        break;
                    }
                }

                if (firstOptionIndex != -1) {
                    questionTextContent = lines.mid(0, firstOptionIndex).join("\n").trimmed().simplified();

                    for (int i = firstOptionIndex; i < lines.size(); ++i) {
                        QRegularExpressionMatch optMatch = optionRegex.match(lines[i]);
                        if (optMatch.hasMatch()) {
                            options.append(optMatch.captured(2).trimmed().simplified());
                        }
                    }
                } else {
                    questionTextContent = lines.join("\n").trimmed();
                }

                if (!questionTextContent.isEmpty()) {
                    int type = options.isEmpty() ? 0 : 1;
                    std::optional<std::vector<std::string>> optVec;

                    if (!options.isEmpty()) {
                        optVec = std::vector<std::string>();
                        for (const QString& opt : options) {
                            optVec->push_back(opt.toStdString());
                        }
                    }

                    auto newQuestion = std::make_shared<Question>(
                        questionTextContent.toStdString(),
                        type,
                        optVec,
                        0,  // default correct option index
                        db->topics[topicsCombo->currentIndex()]
                    );

                    db->addQuestion(newQuestion);
                    ++addedCount;
                }
            }

            if (addedCount > 0) {
                updateQuestionsTable();
                showInfo(QString("Импортировано вопросов: %1").arg(addedCount));
            } else {
                showError("Не удалось распознать ни одного вопроса.");
            }
        }
}

void MainWindow::onQuestionSelectionChanged()
{
    bool hasSelection = !questionsTable->selectedItems().isEmpty();
    editQuestionBtn->setEnabled(hasSelection);
    removeQuestionBtn->setEnabled(hasSelection);
}
void MainWindow::onEditQuestion()
{
    QModelIndexList selection = questionsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        showError("Выберите вопрос для редактирования");
        return;
    }
    
    int row = selection.first().row();
    auto topic = db->topics[topicsCombo->currentIndex()];
    auto questions = db->getQuestionsByTopic(topic);
    
    if (row < 0 || row >= static_cast<int>(questions.size())) {
        return;
    }
    
    auto question = questions[row];
    
    QDialog dialog(this);
    dialog.setWindowTitle("Редактировать вопрос");
    dialog.setMinimumWidth(500);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    // Add auto-parse button at the top
    QPushButton* autoAddBtn = new QPushButton("Автодобавление", &dialog);
    layout->addWidget(autoAddBtn);
    
    QFormLayout* formLayout = new QFormLayout();
    
    QLineEdit* questionText = new QLineEdit(&dialog);
    questionText->setText(QString::fromStdString(question->questionText));
    formLayout->addRow("Текст вопроса:", questionText);
    
    QRadioButton* singleChoice = new QRadioButton("Без вариантов ответа", &dialog);
    QRadioButton* multipleChoice = new QRadioButton("С вариантами ответа", &dialog);
    
    if (question->questionType == 0) {
        singleChoice->setChecked(true);
    } else {
        multipleChoice->setChecked(true);
    }
    
    QHBoxLayout* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(singleChoice);
    typeLayout->addWidget(multipleChoice);
    formLayout->addRow("Тип вопроса:", typeLayout);
    
    QSpinBox* optionCount = new QSpinBox(&dialog);
    optionCount->setMinimum(2);
    optionCount->setMaximum(10);
    optionCount->setValue(question->options ? question->options->size() : 4);
    optionCount->setEnabled(question->questionType == 1);
    formLayout->addRow("Количество вариантов:", optionCount);
    
    QListWidget* optionsList = new QListWidget(&dialog);
    optionsList->setEnabled(question->questionType == 1);
    
    if (question->options) {
        for (size_t i = 0; i < question->options->size(); ++i) {
            QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(question->options->at(i)), optionsList);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    }
    
    QSpinBox* correctOption = new QSpinBox(&dialog);
    correctOption->setMinimum(0);
    correctOption->setMaximum(question->options ? question->options->size() - 1 : 9);
    correctOption->setValue(question->correctOptionIndex >= 0 ? question->correctOptionIndex : 0);
    correctOption->setEnabled(question->questionType == 1);
    formLayout->addRow("Правильный вариант:", correctOption);
    
    connect(autoAddBtn, &QPushButton::clicked, [=]() {
        QDialog autoDialog(this);
        autoDialog.setWindowTitle("Автодобавление вопроса");
        autoDialog.setMinimumWidth(600);
        autoDialog.setMinimumHeight(400);
        
        QVBoxLayout* autoLayout = new QVBoxLayout(&autoDialog);
        QLabel* instructionLabel = new QLabel("Введите текст вопроса и варианты ответов в одном блоке. "
                                             "Программа автоматически распознает варианты, если они помечены "
                                             "буквами (A), Б), 1) и т.д.) или разделены новыми строками.", &autoDialog);
        instructionLabel->setWordWrap(true);
        autoLayout->addWidget(instructionLabel);
        
        QPlainTextEdit* textEdit = new QPlainTextEdit(&autoDialog);
        autoLayout->addWidget(textEdit);
        
        QDialogButtonBox* autoButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &autoDialog);
        connect(autoButtonBox, &QDialogButtonBox::accepted, &autoDialog, &QDialog::accept);
        connect(autoButtonBox, &QDialogButtonBox::rejected, &autoDialog, &QDialog::reject);
        autoLayout->addWidget(autoButtonBox);
        
        if (autoDialog.exec() == QDialog::Accepted) {
            QString text = textEdit->toPlainText().trimmed();
            if (text.isEmpty()) {
                return;
            }
            
            QRegularExpression regex("[A-ZА-Я]\\)|[0-9]\\)");
            bool detectableVariants = text.contains(regex);
            QStringList lines = text.split('\n');
            
            QString questionTextContent;
            QStringList options;
            
            if (detectableVariants) {
                int firstOptionIndex = -1;
                QRegularExpression optionRegex("^\\s*([A-ZА-Яa-zа-я0-9])\\)\\s*(.*)$");
                
                for (int i = 0; i < lines.size(); i++) {
                    QRegularExpressionMatch match = optionRegex.match(lines[i]);
                    if (match.hasMatch()) {
                        firstOptionIndex = i;
                        break;
                    }
                }
                
                if (firstOptionIndex != -1) {
                    questionTextContent = lines.mid(0, firstOptionIndex).join("\n").trimmed();
                    
                    for (int i = firstOptionIndex; i < lines.size(); i++) {
                        QRegularExpressionMatch match = optionRegex.match(lines[i]);
                        if (match.hasMatch()) {
                            options.append(match.captured(2).trimmed());
                        }
                    }
                } else {
                    questionTextContent = lines.first().trimmed();
                    
                    if (lines.size() > 1) {
                        options = lines.mid(1);
                        for (int i = 0; i < options.size(); i++) {
                            options[i] = options[i].trimmed();
                        }
                    }
                }
            } else {
                questionTextContent = lines.first().trimmed();
                
                if (lines.size() > 1) {
                    options = lines.mid(1);
                    for (int i = 0; i < options.size(); i++) {
                        options[i] = options[i].trimmed();
                    }
                }
            }
            
            questionText->setText(questionTextContent);
            
            if (!options.isEmpty()) {
                multipleChoice->setChecked(true);
                optionCount->setValue(options.size());
                
                optionsList->clear();
                for (int i = 0; i < options.size(); i++) {
                    QListWidgetItem* item = new QListWidgetItem(options[i], optionsList);
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                }
            }
        }
    });
    
    connect(multipleChoice, &QRadioButton::toggled, [=](bool checked) {
        optionCount->setEnabled(checked);
        optionsList->setEnabled(checked);
        correctOption->setEnabled(checked);
        
        if (checked && optionsList->count() == 0) {
            optionsList->clear();
            for (int i = 0; i < optionCount->value(); ++i) {
                QListWidgetItem* item = new QListWidgetItem("Вариант " + QString::number(i+1), optionsList);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            }
        }
    });
    
    connect(optionCount, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        int currentCount = optionsList->count();
        
        if (value > currentCount) {
            for (int i = currentCount; i < value; ++i) {
                QListWidgetItem* item = new QListWidgetItem("Вариант " + QString::number(i+1), optionsList);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            }
        } else if (value < currentCount) {
            for (int i = currentCount - 1; i >= value; --i) {
                delete optionsList->item(i);
            }
        }
        
        correctOption->setMaximum(value - 1);
        if (correctOption->value() >= value) {
            correctOption->setValue(value - 1);
        }
    });
    
    layout->addLayout(formLayout);
    
    QLabel* optionsLabel = new QLabel("Варианты ответа:", &dialog);
    layout->addWidget(optionsLabel);
    layout->addWidget(optionsList);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString cleanedQuestionText = questionText->text().simplified();
        if (cleanedQuestionText.isEmpty()) {
            showError("Текст вопроса не может быть пустым");
            return;
        }
        
        int questionType = multipleChoice->isChecked() ? 1 : 0;
        std::optional<std::vector<std::string>> options;
        int correctOptionIndex = -1;
        
        if (questionType == 1) {
            options = std::vector<std::string>();
            for (int i = 0; i < optionsList->count(); ++i) {
                QString optionText = optionsList->item(i)->text().simplified();
                if (optionText.isEmpty()) {
                    showError("Вариант ответа не может быть пустым");
                    return;
                }
                options->push_back(optionText.toStdString());
            }
            correctOptionIndex = correctOption->value();
        } else {
            options = std::nullopt;
        }
        
        auto updatedQuestion = std::make_shared<Question>(
            cleanedQuestionText.toStdString(),
            questionType,
            options,
            correctOptionIndex,
            db->topics[topicsCombo->currentIndex()]
        );
        
        db->editQuestion(question, updatedQuestion);
        updateQuestionsTable();
        showInfo("Вопрос успешно обновлен");
    }
}
void MainWindow::setupMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("Файл");
    
    QAction* saveAction = new QAction("Сохранить базу вопросов", this);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveDatabase);
    fileMenu->addAction(saveAction);
    
    QAction* loadAction = new QAction("Загрузить базу вопросов", this);
    connect(loadAction, &QAction::triggered, this, &MainWindow::onLoadDatabase);
    fileMenu->addAction(loadAction);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = new QAction("Выход", this);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    
    QMenu* helpMenu = menuBar()->addMenu("Справка");
    QAction* aboutAction = new QAction("О программе", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);
}

void MainWindow::updateTopicsCombo()
{
    topicsCombo->blockSignals(true);
    topicsCombo->clear();
    
    for (const auto& topic : db->topics) {
        topicsCombo->addItem(QString::fromStdString(topic->name));
    }
    
    topicsCombo->blockSignals(false);
    
    bool hasTopics = topicsCombo->count() > 0;
    removeTopicBtn->setEnabled(hasTopics);
    addQuestionBtn->setEnabled(hasTopics);
    generateQuizBtn->setEnabled(hasTopics);
    
    if (hasTopics) {
        onTopicChanged(topicsCombo->currentIndex());
    } else {
        questionsTable->setRowCount(0);
        removeQuestionBtn->setEnabled(false);
    }
}

void MainWindow::updateQuestionsTable()
{
    questionsTable->setRowCount(0);
    
    if (topicsCombo->currentIndex() < 0) {
        return;
    }
    
    auto topic = db->topics[topicsCombo->currentIndex()];
    auto questions = db->getQuestionsByTopic(topic);
    
    questionsTable->setRowCount(questions.size());
    
    for (size_t i = 0; i < questions.size(); ++i) {
        const auto& question = questions[i];
        
        QTableWidgetItem* textItem = new QTableWidgetItem(QString::fromStdString(question->questionText));
        questionsTable->setItem(i, 0, textItem);
        
        QString typeText = question->questionType == 0 ? "Без вариантов" : "С вариантами";
        QTableWidgetItem* typeItem = new QTableWidgetItem(typeText);
        questionsTable->setItem(i, 1, typeItem);
        
        QString optionsText;
        if (question->options) {
            const auto& options = *(question->options);
            for (size_t j = 0; j < options.size(); ++j) {
                if (j > 0) optionsText += ", ";
                optionsText += QString::fromStdString(options[j]);
                if (j == question->correctOptionIndex) {
                    optionsText += " ✓";
                }
            }
        }
        QTableWidgetItem* optionsItem = new QTableWidgetItem(optionsText);
        questionsTable->setItem(i, 2, optionsItem);
    }
    
    questionsTable->resizeColumnsToContents();
    removeQuestionBtn->setEnabled(!questions.empty());
}

void MainWindow::onTopicChanged(int index)
{
    if (index >= 0) {
        updateQuestionsTable();
        batchImportBtn->setEnabled(true);
    }
}

void MainWindow::onAddTopic()
{
    bool ok;
    QString topicName = QInputDialog::getText(this, "Добавить тему", 
                                             "Название темы:", QLineEdit::Normal, 
                                             "", &ok).simplified();
    if (ok && !topicName.isEmpty()) {
        for (const auto& topic : db->topics) {
            if (topic->name == topicName.toStdString()) {
                showError("Тема с таким названием уже существует");
                return;
            }
        }
        
        auto newTopic = std::make_shared<Topic>(topicName.toStdString());
        db->topics.push_back(newTopic);
        updateTopicsCombo();
        topicsCombo->setCurrentIndex(topicsCombo->count() - 1);
        showInfo("Тема успешно добавлена");
    }
}

void MainWindow::onRemoveTopic()
{
    if (topicsCombo->currentIndex() < 0) {
        return;
    }
    
    auto topic = db->topics[topicsCombo->currentIndex()];
    
    if (!confirm("Вы действительно хотите удалить тему '" + 
                QString::fromStdString(topic->name) + "'?")) {
        return;
    }
    
    auto it = std::remove_if(db->questions.begin(), db->questions.end(),
                           [&topic](const std::shared_ptr<Question>& q) {
                               return q->topic->name == topic->name;
                           });
    db->questions.erase(it, db->questions.end());
    
    db->topics.erase(db->topics.begin() + topicsCombo->currentIndex());
    
    updateTopicsCombo();
    showInfo("Тема успешно удалена");
}

void MainWindow::onAddQuestion()
{
    if (topicsCombo->currentIndex() < 0) {
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить вопрос");
    dialog.setMinimumWidth(500);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QPushButton* autoAddBtn = new QPushButton("Автодобавление", &dialog);
    layout->addWidget(autoAddBtn);
    
    QFormLayout* formLayout = new QFormLayout();
    
    QLineEdit* questionText = new QLineEdit(&dialog);
    formLayout->addRow("Текст вопроса:", questionText);
    
    QRadioButton* singleChoice = new QRadioButton("Без вариантов ответа", &dialog);
    QRadioButton* multipleChoice = new QRadioButton("С вариантами ответа", &dialog);
    singleChoice->setChecked(true);
    
    QHBoxLayout* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(singleChoice);
    typeLayout->addWidget(multipleChoice);
    formLayout->addRow("Тип вопроса:", typeLayout);
    
    QSpinBox* optionCount = new QSpinBox(&dialog);
    optionCount->setMinimum(2);
    optionCount->setMaximum(10);
    optionCount->setValue(4);
    optionCount->setEnabled(false);
    formLayout->addRow("Количество вариантов:", optionCount);
    
    QListWidget* optionsList = new QListWidget(&dialog);
    optionsList->setEnabled(false);
    
    QSpinBox* correctOption = new QSpinBox(&dialog);
    correctOption->setMinimum(0);
    correctOption->setMaximum(9);
    correctOption->setValue(0);
    correctOption->setEnabled(false);
    formLayout->addRow("Правильный вариант:", correctOption);
    
    connect(autoAddBtn, &QPushButton::clicked, [=]() {
        QDialog autoDialog(this);
        autoDialog.setWindowTitle("Автодобавление вопроса");
        autoDialog.setMinimumWidth(600);
        autoDialog.setMinimumHeight(400);
        
        QVBoxLayout* autoLayout = new QVBoxLayout(&autoDialog);
        QLabel* instructionLabel = new QLabel("Введите текст вопроса и варианты ответов в одном блоке. "
                                             "Программа автоматически распознает варианты, если они помечены "
                                             "буквами (A), Б), 1) и т.д.) или разделены новыми строками.", &autoDialog);
        instructionLabel->setWordWrap(true);
        autoLayout->addWidget(instructionLabel);
        
        QPlainTextEdit* textEdit = new QPlainTextEdit(&autoDialog);
        autoLayout->addWidget(textEdit);
        
        QDialogButtonBox* autoButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &autoDialog);
        connect(autoButtonBox, &QDialogButtonBox::accepted, &autoDialog, &QDialog::accept);
        connect(autoButtonBox, &QDialogButtonBox::rejected, &autoDialog, &QDialog::reject);
        autoLayout->addWidget(autoButtonBox);
        
        if (autoDialog.exec() == QDialog::Accepted) {
            QString text = textEdit->toPlainText().trimmed();
            if (text.isEmpty()) {
                return;
            }
            
            QRegularExpression regex("[A-ZА-Я]\\)|[0-9]\\)");
            bool detectableVariants = text.contains(regex);
            QStringList lines = text.split('\n');
            
            QString questionTextContent;
            QStringList options;
            
            if (detectableVariants) {
                int firstOptionIndex = -1;
                QRegularExpression optionRegex("^\\s*([A-ZА-Яa-zа-я0-9])\\)\\s*(.*)$");
                
                for (int i = 0; i < lines.size(); i++) {
                    if (optionRegex.match(lines[i]).hasMatch()) {
                        firstOptionIndex = i;
                        break;
                    }
                }
                
                if (firstOptionIndex != -1) {
                    questionTextContent = lines.mid(0, firstOptionIndex).join("\n").trimmed();
                    
                    for (int i = firstOptionIndex; i < lines.size(); i++) {
                        if (optionRegex.match(lines[i]).hasMatch()) {
                            QRegularExpressionMatch match = optionRegex.match(lines[i]);
                            if (match.hasMatch()) {
                                options.append(match.captured(2).trimmed());
                            }
                        }
                    }
                } else {
                    questionTextContent = lines.first().trimmed();
                    
                    if (lines.size() > 1) {
                        options = lines.mid(1);
                        for (int i = 0; i < options.size(); i++) {
                            options[i] = options[i].trimmed();
                        }
                    }
                }
            } else {

                questionTextContent = lines.first().trimmed();
                
                if (lines.size() > 1) {
                    options = lines.mid(1);
                    for (int i = 0; i < options.size(); i++) {
                        options[i] = options[i].trimmed();
                    }
                }
            }
            
            questionText->setText(questionTextContent);
            
            if (!options.isEmpty()) {
                multipleChoice->setChecked(true);
                optionCount->setValue(options.size());
                
                optionsList->clear();
                for (int i = 0; i < options.size(); i++) {
                    QListWidgetItem* item = new QListWidgetItem(options[i], optionsList);
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                }
            }
        }
    });
    
    connect(multipleChoice, &QRadioButton::toggled, [=](bool checked) {
        optionCount->setEnabled(checked);
        optionsList->setEnabled(checked);
        correctOption->setEnabled(checked);
        
        if (checked) {
            optionsList->clear();
            for (int i = 0; i < optionCount->value(); ++i) {
                QListWidgetItem* item = new QListWidgetItem("Вариант " + QString::number(i+1), optionsList);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            }
        }
    });
    
    connect(optionCount, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        int currentCount = optionsList->count();
        
        if (value > currentCount) {
            for (int i = currentCount; i < value; ++i) {
                QListWidgetItem* item = new QListWidgetItem("Вариант " + QString::number(i+1), optionsList);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            }
        } else if (value < currentCount) {
            for (int i = currentCount - 1; i >= value; --i) {
                delete optionsList->item(i);
            }
        }
        
        correctOption->setMaximum(value - 1);
        if (correctOption->value() >= value) {
            correctOption->setValue(value - 1);
        }
    });
    
    layout->addLayout(formLayout);
    
    QLabel* optionsLabel = new QLabel("Варианты ответа:", &dialog);
    layout->addWidget(optionsLabel);
    layout->addWidget(optionsList);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString cleanedQuestionText = questionText->text().simplified();
        if (cleanedQuestionText.isEmpty()) {
            showError("Текст вопроса не может быть пустым");
            return;
        }
        
        int questionType = multipleChoice->isChecked() ? 1 : 0;
        std::optional<std::vector<std::string>> options;
        int correctOptionIndex = -1;
        
        if (questionType == 1) {
            options = std::vector<std::string>();
            for (int i = 0; i < optionsList->count(); ++i) {
                QString optionText = optionsList->item(i)->text().simplified();
                if (optionText.isEmpty()) {
                    showError("Вариант ответа не может быть пустым");
                    return;
                }
                options->push_back(optionText.toStdString());
            }
            correctOptionIndex = correctOption->value();
        } else {
            options = std::nullopt;
        }
        
        auto newQuestion = std::make_shared<Question>(
            questionText->text().toStdString(),
            questionType,
            options,
            correctOptionIndex,
            db->topics[topicsCombo->currentIndex()]
        );
        
        db->addQuestion(newQuestion);
        updateQuestionsTable();
        showInfo("Вопрос успешно добавлен");
    }
}

void MainWindow::onRemoveQuestion()
{
    QModelIndexList selection = questionsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        showError("Выберите вопрос для удаления");
        return;
    }
    
    if (!confirm("Вы действительно хотите удалить выбранный вопрос?")) {
        return;
    }
    
    int row = selection.first().row();
    auto topic = db->topics[topicsCombo->currentIndex()];
    auto questions = db->getQuestionsByTopic(topic);
    
    if (row >= 0 && row < static_cast<int>(questions.size())) {
        db->removeQuestion(questions[row]);
        updateQuestionsTable();
        showInfo("Вопрос успешно удален");
    }
}

void MainWindow::onGenerateQuiz()
{
    if (db->topics.empty()) {
        showError("Нет доступных тем для создания теста");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Создать тест");
    dialog.setMinimumWidth(500);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QTableWidget* topicsTable = new QTableWidget(&dialog);
    topicsTable->setColumnCount(4); 
    topicsTable->setHorizontalHeaderLabels({"Включить", "Тема", "Вопросов в теме", "Количество в тесте"});
    topicsTable->horizontalHeader()->setStretchLastSection(true);
    topicsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    topicsTable->setRowCount(db->topics.size());
    for (size_t i = 0; i < db->topics.size(); ++i) {
        const auto& topic = db->topics[i];
        auto questions = db->getQuestionsByTopic(topic);
        
        QWidget* checkboxWidget = new QWidget();
        QHBoxLayout* checkboxLayout = new QHBoxLayout(checkboxWidget);
        QCheckBox* checkbox = new QCheckBox();
        checkbox->setChecked(true);
        checkboxLayout->addWidget(checkbox);
        checkboxLayout->setAlignment(Qt::AlignCenter);
        checkboxLayout->setContentsMargins(0, 0, 0, 0);
        topicsTable->setCellWidget(i, 0, checkboxWidget);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(topic->name));
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        topicsTable->setItem(i, 1, nameItem);
        
        QTableWidgetItem* countItem = new QTableWidgetItem(QString::number(questions.size()));
        countItem->setFlags(countItem->flags() & ~Qt::ItemIsEditable);
        topicsTable->setItem(i, 2, countItem);
        
        QSpinBox* spinBox = new QSpinBox(&dialog);
        spinBox->setMinimum(0);
        spinBox->setMaximum(questions.size());
        spinBox->setValue(questions.empty() ? 0 : std::min(5, static_cast<int>(questions.size())));
        topicsTable->setCellWidget(i, 3, spinBox);
        
        connect(checkbox, &QCheckBox::toggled, [spinBox, questions](bool checked) {
            spinBox->setEnabled(checked);
            if (!checked) {
                spinBox->setValue(0);
            } else if (spinBox->value() == 0 && !questions.empty()) {
                spinBox->setValue(std::min(5, static_cast<int>(questions.size())));
            }
        });
    }
    
    topicsTable->resizeColumnsToContents();
    layout->addWidget(new QLabel("Выберите темы и количество вопросов:", &dialog));
    layout->addWidget(topicsTable);
    
    QSpinBox* variantCount = new QSpinBox(&dialog);
    variantCount->setMinimum(1);
    variantCount->setMaximum(100);
    variantCount->setValue(1);
    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow("Количество вариантов:", variantCount);
    
    // QCheckBox* shuffleQuestions = new QCheckBox("Перемешать вопросы", &dialog);
    // shuffleQuestions->setChecked(true);
    // formLayout->addRow("", shuffleQuestions);
    
    layout->addLayout(formLayout);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString saveDir = QFileDialog::getExistingDirectory(this, 
            "Выберите папку для сохранения тестов", 
            QDir::homePath(), 
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        
        if (saveDir.isEmpty()) {
            showInfo("Создание тестов отменено");
            return;
        }
        
        std::vector<std::pair<std::shared_ptr<Topic>, int>> selectedTopics;
        int totalQuestions = 0;
        
        for (size_t i = 0; i < db->topics.size(); ++i) {
            QWidget* checkboxWidget = topicsTable->cellWidget(i, 0);
            QCheckBox* checkbox = checkboxWidget->findChild<QCheckBox*>();
            
            if (checkbox->isChecked()) {
                QSpinBox* spinBox = qobject_cast<QSpinBox*>(topicsTable->cellWidget(i, 3));
                int count = spinBox->value();
                
                if (count > 0) {
                    selectedTopics.emplace_back(db->topics[i], count);
                    totalQuestions += count;
                }
            }
        }
        
        if (selectedTopics.empty() || totalQuestions == 0) {
            showError("Выберите хотя бы один вопрос для включения в тест");
            return;
        }
        
        int variants = variantCount->value();
        bool shuffle = false;
        
        for (int i = 0; i < variants; ++i) {
            auto quizVariant = std::make_shared<QuizVariant>("Вариант " + std::to_string(i + 1));
            
            for (const auto& topicPair : selectedTopics) {
                auto topic = topicPair.first;
                int questionCount = topicPair.second;
                auto questions = db->getQuestionsByTopic(topic);
                
                std::shuffle(questions.begin(), questions.end(), std::mt19937(std::random_device{}()));
                
                for (int j = 0; j < std::min(questionCount, static_cast<int>(questions.size())); ++j) {
                    quizVariant->addQuestion(questions[j]);
                }
            }
            
            if (shuffle) {
                quizVariant->shuffleQuestions();
            }
            
            QString fileName = saveDir + "/" + QDate::currentDate().toString("yyyy-MM-dd") + "_" + QString::fromStdString(selectedTopics[0].first->name) + "_variant_" + QString::number(i + 1) + ".html";
            try {
                db->writeExamToDoc(fileName.toStdString(), quizVariant);
            } catch (const std::exception& e) {
                showError(QString("Ошибка при сохранении варианта теста: ") + e.what());
                continue;
            }
        }
        
        showInfo(QString("Успешно создано %1 вариантов теста в папке %2").arg(variants).arg(saveDir));
    }
}

void MainWindow::onSaveDatabase()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить базу вопросов", 
                                                  "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    
    try {
        db->writeQuestionsToFile(fileName.toStdString());
        showInfo("База вопросов успешно сохранена");
    } catch (const std::exception& e) {
        showError(QString("Ошибка при сохранении базы вопросов: ") + e.what());
    }
}

void MainWindow::onLoadDatabase()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Загрузить базу вопросов", 
                                                 "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    
    try {
        db->readQuestionsFromFile(fileName.toStdString());
        updateTopicsCombo();
        showInfo("База вопросов успешно загружена");
    } catch (const std::exception& e) {
        showError(QString("Ошибка при загрузке базы вопросов: ") + e.what());
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "О программе", 
                     "MadExam - Система создания тестов\n\n"
                     "Версия 1.2\n"
                     "© 2025 Тургунов Мади");
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Ошибка", message);
}

void MainWindow::showInfo(const QString& message)
{
    statusBar->showMessage(message, 3000);
}

bool MainWindow::confirm(const QString& message)
{
    return QMessageBox::question(this, "Подтверждение", message, 
                               QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}
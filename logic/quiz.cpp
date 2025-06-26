#include "quiz.h"
#include <fstream>
#include <algorithm>
#include "docwriter.h"
#include <random>
#include <iostream>
void QuestionDatabase::addQuestion(std::shared_ptr<Question> question) {
    questions.push_back(question);
}
void QuestionDatabase::removeQuestion(const std::shared_ptr<Question>& question) {
    auto it = std::remove(questions.begin(), questions.end(), question);
    if (it != questions.end()) {
        questions.erase(it, questions.end());
    }
}
std::vector<std::shared_ptr<Question>> QuestionDatabase::getQuestionsByTopic(const std::shared_ptr<Topic>& topic) const {
    std::vector<std::shared_ptr<Question>> result;
    for (const auto& question : questions) {
        if (question->topic->name == topic->name) {  
            result.push_back(question);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Question>> QuestionDatabase::getAllQuestions() const {
    return questions;
}

void QuestionDatabase::updateQuestion(const std::shared_ptr<Question>& oldQuestion, const std::shared_ptr<Question>& newQuestion){
    auto it = std::find(questions.begin(), questions.end(), oldQuestion);
    if (it != questions.end()) {
        *it = newQuestion;
    }
}
std::shared_ptr<Question> QuestionDatabase::getQuestionByIndex(int index) const{
    if (index < 0 || index >= static_cast<int>(questions.size())) {
        throw std::out_of_range("Index out of range");
    }
    return questions[index];
}
int QuestionDatabase::getQuestionCount() const {
    return static_cast<int>(questions.size());
}



void QuestionDatabase::writeQuestionsToFile(const std::string& filePath) const {
    std::ofstream outFile(filePath);
    if (!outFile) {
        throw std::runtime_error("Could not open file for writing: " + filePath);
    }
    for (const auto& question : questions) {
        outFile << question->questionText << "\n"
                << question->questionType << "\n"
                << (question->options ? std::to_string(question->options->size()) : "0") << "\n";
        if (question->options) {
            for (const auto& option : *question->options) {
                outFile << option << "\n";
            }
        }
        outFile << question->correctOptionIndex << "\n"
                << question->topic->name << "\n";
    }
    outFile.close();
}
std::vector<std::shared_ptr<Question>> QuestionDatabase::readQuestionsFromFile(const std::string& filePath){
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::ofstream outFile(filePath);
        if (!outFile) {
            throw std::runtime_error("Could not create file: " + filePath);
        }
        outFile.close();
        return std::vector<std::shared_ptr<Question>>();
    }
    std::vector<std::shared_ptr<Question>> loadedQuestions;
    std::string line;
    while (std::getline(inFile, line)) {
        std::string questionText = line;
        int questionType;
        std::cout << "Reading question: " << questionText << std::endl;
        std::getline(inFile, line);
        questionType = std::stoi(line);
        std::cout << "Question type: " << questionType << std::endl;
        std::optional<std::vector<std::string>> options;
        std::getline(inFile, line);
        std::cout << "Number of options: " << line << std::endl;
        int optionCount = std::stoi(line);
        if (optionCount > 0) {
            options = std::vector<std::string>();
            for (int i = 0; i < optionCount; ++i) {
                std::getline(inFile, line);
                options->push_back(line);  
            }
        } else {
            options = std::nullopt;
        }
        std::getline(inFile, line);
        int correctOptionIndex = std::stoi(line);
        std::cout << "Correct option index: " << correctOptionIndex << std::endl;
        std::getline(inFile, line);
        std::shared_ptr<Topic> topic;
        auto it = std::find_if(topics.begin(), topics.end(), [&line](const std::shared_ptr<Topic>& t) {
            return t->name == line;
        });

        if (it != topics.end()) {
            std::cout << "Using existing topic: " << line << std::endl;
            topic = *it; 
        } else {
            std::cout << "Creating new topic: " << line << std::endl;
            topic = std::make_shared<Topic>(line);
            topics.push_back(topic);
        }

        auto question = std::make_shared<Question>(questionText, questionType, options, correctOptionIndex, topic);
        loadedQuestions.push_back(question);
    }
    inFile.close();
    for (const auto& loadedQuestion : loadedQuestions) {
        auto it = std::find(questions.begin(), questions.end(), loadedQuestion);
        if (it == questions.end()) {
            questions.push_back(loadedQuestion);
        } else {
            std::cout << "Question already exists in the database: " << loadedQuestion->questionText << std::endl;
        }
    }
    topics = generateTopicsFromQuestions();
    return loadedQuestions;
}

std::vector<std::shared_ptr<Topic>> QuestionDatabase::generateTopicsFromQuestions() const {
    std::vector<std::shared_ptr<Topic>> generatedTopics;
    for (const auto& question : questions) {
        auto it = std::find_if(generatedTopics.begin(), generatedTopics.end(), 
            [&question](const std::shared_ptr<Topic>& topic) {
                return topic->name == question->topic->name;
            });

        if (it == generatedTopics.end()) {
            generatedTopics.push_back(question->topic);
        }
    }
    
    std::sort(generatedTopics.begin(), generatedTopics.end(), 
        [](const std::shared_ptr<Topic>& a, const std::shared_ptr<Topic>& b) {
            return a->name < b->name;
        });
    
    generatedTopics.erase(std::unique(generatedTopics.begin(), generatedTopics.end(),
        [](const std::shared_ptr<Topic>& a, const std::shared_ptr<Topic>& b) {
            return a->name == b->name;
        }), generatedTopics.end());
        
    return generatedTopics;
}

void QuestionDatabase::writeExamToDoc(const std::string& filePath, const std::shared_ptr<QuizVariant>& quizVariant) const {
    DocumentWriter docWriter;
    if (!docWriter.createDocument(filePath, quizVariant)) {
        throw std::runtime_error("Failed to create document: " + filePath);
    }
}


void QuizVariant::addQuestion(std::shared_ptr<Question> question) {
    questions.push_back(question);
}

void QuestionDatabase::editQuestion(const std::shared_ptr<Question>& oldQuestion, const std::shared_ptr<Question>& newQuestion) {
    auto it = std::find(questions.begin(), questions.end(), oldQuestion);
    if (it != questions.end()) {
        *it = newQuestion;
    } else {
        throw std::runtime_error("Question not found in the variant");
    }
}

void QuizVariant::removeQuestion(const std::shared_ptr<Question>& question) {
    auto it = std::remove(questions.begin(), questions.end(), question);
    if (it != questions.end()) {
        questions.erase(it, questions.end());
    }
}

std::vector<std::shared_ptr<Question>> QuizVariant::getQuestions() const {
    return questions;
}

void QuizVariant::shuffleQuestions() {
    std::shuffle(questions.begin(), questions.end(), std::mt19937(std::random_device{}()));
}
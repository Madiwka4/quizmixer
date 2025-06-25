#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "logic/quiz.h"
class CLI {
    private:
        std::shared_ptr<QuestionDatabase> db;
    public:
        CLI(std::shared_ptr<QuestionDatabase> database);
        void run();
        void processCommand(const std::string& command);
        int selectedTopicIndex = -1; // Index of the currently selected topic, -1 means no topic is selected
    private:
        void showHelp();
        void addTopic();
        void removeTopic();
        void selectTopic();
        void listTopics();
        void addQuestion();
        void removeQuestion();
        void generateQuiz();
        void listQuestions();
        void exit();
};
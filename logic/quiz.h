#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>

class Topic{
    public:
        std::string name;

        Topic(const std::string& topicName) : name(topicName) {}
};

class Question {
    public:
        std::string questionText;
        int questionType;
        std::optional<std::vector<std::string>> options;
        int correctOptionIndex;
        std::shared_ptr<Topic> topic;

        Question(const std::string& text, int type, const std::optional<std::vector<std::string>>& opts, int correctIndex, std::shared_ptr<Topic> topicPtr)
            : questionText(text), questionType(type), options(opts), correctOptionIndex(correctIndex), topic(topicPtr) {}
};
class QuizVariant {
    public:
        std::string variantName;
        std::vector<std::shared_ptr<Question>> questions;

        QuizVariant(const std::string& name) : variantName(name) {}

        void addQuestion(std::shared_ptr<Question> question);
        void removeQuestion(const std::shared_ptr<Question>& question);
        std::vector<std::shared_ptr<Question>> getQuestions() const;
        void shuffleQuestions();
};
class QuestionDatabase{
    public:
        std::vector<std::shared_ptr<Question>> questions;
        std::vector<std::shared_ptr<Topic>> topics;


        void addQuestion(std::shared_ptr<Question> question);
        void editQuestion(const std::shared_ptr<Question>& oldQuestion, const std::shared_ptr<Question>& newQuestion);
        std::vector<std::shared_ptr<Question>> getQuestionsByTopic(const std::shared_ptr<Topic>& topic) const;
        std::vector<std::shared_ptr<Question>> readQuestionsFromFile(const std::string& filePath);
        void writeQuestionsToFile(const std::string& filePath) const;
        std::vector<std::shared_ptr<Question>> getAllQuestions() const;
        void removeQuestion(const std::shared_ptr<Question>& question);
        void updateQuestion(const std::shared_ptr<Question>& oldQuestion, const std::shared_ptr<Question>& newQuestion);
        std::shared_ptr<Question> getQuestionByIndex(int index) const;
        int getQuestionCount() const;
        void writeExamToDoc(const std::string& filePath, const std::shared_ptr<QuizVariant>& QuizVariant) const;
    private:
        std::vector<std::shared_ptr<Topic>> generateTopicsFromQuestions() const;

};


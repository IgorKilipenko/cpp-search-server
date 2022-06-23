#ifndef __SERVER_TEST_H__
#define __SERVER_TEST_H__

#include "../server.hpp"

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent();

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer();

// --------- Окончание модульных тестов поисковой системы -----------

#endif // __SERVER_TEST_H__

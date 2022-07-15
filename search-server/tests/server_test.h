#pragma once

#include "../search_server.h"

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent();

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer();

// --------- Окончание модульных тестов поисковой системы -----------


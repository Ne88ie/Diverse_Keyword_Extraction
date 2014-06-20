Diverse_Keyword_Extraction
==========================
Весенняя практика 2014 в ЦРТ. Идея алгоритма получена из статьи "Diverse Keyword Extraction from Conversations" http://aclweb.org/anthology//P/P13/P13-2115.pdf
==========================
В папке data/results находятся по три файла на каждый эксперимент:

"Имя_базы_top_words_log_info.txt" – top_words это наиболее характерные слова для каждой темы, получаемые при TopicModeling

"Имя_базы_doc-topics_log_info.txt" – doc-topics это распределение тем по каждому документу, получаемое при TopicModeling

"Имя_базы_doc-keywords_log_info.txt" – doc-keywords это ключевые слова для каждого документа, получаемые при Diverse_Keyword_Extraction

"_log_info" это информация о количестве тем, которые выделял TopicModeling из текстов, о том была ли нормализация слов и исключались ли стоп-слова из текста при обработке

А также в папке data/results содержится по файлу на базу, в котором находится она в сконвертированном виде, выглядят так:
"Имя_базы_convert_out.txt"

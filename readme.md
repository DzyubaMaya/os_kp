## Курсовая работа по курсу Операционные системы

### Цель курсового проекта

1. Приобретение практических навыков в использовании знаний, полученных в течении курса
2. Проведение исследования в выбранной предметной области

### Задание
Необходимо спроектировать и реализовать программный прототип в соответствии с выбранным вариантом. Произвести анализ и сделать вывод на основании данных, полученных при работе программного прототипа.

### прототип 
Создание клиента для передачи мгновенных личных сообщений
На основе любой из выбранных технологий:
* Pipes
* Sockets
* Сервера очередей
* И другие

### вариант 
Клиент-серверная система для передачи мгновенных сообщений. Базовый функционал должен быть следующим:
•  Клиент может присоединиться к серверу, введя логин
•  Клиент может отправить сообщение другому клиенту по его логину
•  Клиент в реальном времени принимает сообщения от других клиентов

Необходимо предусмотреть возможность отправки отложенного сообщения или себе, или другому пользователю. При выходе с сервера отправка всё равно должна осуществиться. Связь между сервером и клиентом должна быть реализована при помощи pipe'ов

### Решение

Будем использовать библиотеку httplib для построения клиентских и серверных приложений, использующих протокол HTTP поверх Socket
Билиотека: https://github.com/yhirose/cpp-httplib

Для скачивания зависимых бибилиотек делаем:
```
git config --global --add safe.directory '*'
```

## проверочные запросы к серверу 

### Тест

```
curl -X POST http://localhost:8080/users \
-H "Content-Type: application/json" \
-d '{"login": "John Doe", "password": "qwerty"}'

curl -X POST http://localhost:8080/users \
-H "Content-Type: application/json" \
-d '{"login": "Jane Doe", "password": "1245678"}'

curl -X GET http://localhost:8080/users

curl -X POST http://localhost:8080/session \
-H "Content-Type: application/json" \
-d '{"login": "John Doe", "password": "qwerty"}'

curl -X POST http://localhost:8080/session \
-H "Content-Type: application/json" \
-d '{"login": "Jane Doe", "password": "1245678"}'

curl -X POST http://localhost:8080/messages \
-H "Content-Type: application/json" \
-d '{"session" : 1 , "to": "Jane Doe", "message": "Hello Jane!"}'

curl -X POST http://localhost:8080/messages \
-H "Content-Type: application/json" \
-d '{"session" : 1 , "to": "Jane Doe", "message": "Lets go to Operating systems exam!"}'

curl -X GET 'http://localhost:8080/messages?session=2&all=true'

curl -X POST http://localhost:8080/messages \
-H "Content-Type: application/json" \
-d '{"session" : 1 , "to": "Jane Doe", "message": "See you later"}'

curl -X GET 'http://localhost:8080/messages?session=2&all=false'
```
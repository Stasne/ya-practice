FROM gcc:11.3 as build

RUN apt update && \
    apt install -y \
    python3-pip \
    cmake &&\
    pip3 install conan\<2.0

# копируем conanfile.txt в контейнер и запускаем conan install
COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing -s build_type=Release -s compiler.libcxx=libstdc++11

# только после этого копируем остальные иходники
COPY ./src /app/src
COPY ./data /app/data

COPY CMakeLists.txt /app/

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF .. && \
    cmake --build . 


# Второй контейнер в том же докерфайле
FROM ubuntu:22.04 as run
# Создадим пользователя www
RUN groupadd -r www && useradd -r -g www www
USER www
# Скопируем приложение со сборочного контейнера в директорию /app.
# Не забываем также папку data, она пригодится.
COPY --from=build /app/build/game_server /app/
COPY ./data /app/data
COPY ./static /app/static
# Запускаем игровой сервер
#--randomize-spawn-points 
#--tick-period=50
ENTRYPOINT ["/app/game_server", "--config-file=/app/data/config.json", "--www-root=/app/static",  "--randomize-spawn-points", "--save-state-period=2000", "--state-file=/tmp/save.save" ] 
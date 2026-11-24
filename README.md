# Система донаведення боєприпасів БПЛА (ДЗ №9: Тести ядра й довершена межа)

Наскрізний проєкт курсу: розділення системи розрахунку балістики та вибору боєприпасів на статичну бібліотеку ядра (`core`), CLI-застосунок (`app`) та тестовий комплект GoogleTest (`unit_test`).

## 🏗️ Архітектура проєкту (Межа «Ядро / Адаптери»)

В ітерації ДЗ №9 доведено до кінця розділення межі між ядром розрахунку та вводом-виводом:
- **`core`** (статична бібліотека):
  - Придатна до виклику без файлової системи!
  - `parseScene(std::istream& in, Scene& out_scene)` — парсинг вхідних даних із довільного текстового потоку `std::istream`.
  - `formatResult(std::ostream& out, const Sol_Candidate& solution)` — форматування результатів у довільний потік `std::ostream`.
  - `calculate_best_solution(...)` — чистий розрахунок точки влучання, точки скиду та вибору боєприпасу.
  - **Ядро не містить жодного файлового інклюду (`<fstream>`) і не виконує файлового I/O чи друку у консоль.**

- **`app`** (CLI-застосунок `main.cpp`):
  - Тонкий адаптер оточення.
  - Розбір аргументів командного рядка (`--input-file`, `--output-file`, `--trajectory`).
  - Відкриття/закриття файлів `std::ifstream` та `std::ofstream`.
  - Вимірювання часу виконання (`elapsed_us`).
  - Обробка помилок входу/порожнього сценарію (вивід `empty scenario` у `std::cerr` з кодом `1`).

---

## 🛠️ Інструкція зі збірки проєкту (CMake)

### 1. Збірка у режимі Debug (Налагодження з автоматичним запуском тестів)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### 2. Збірка у режимі Release (Реліз)
```bash
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

### 3. Діагностична збірка з Санітайзерами (ASan + UBSan)
```bash
cmake -B build-san -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build-san
```

---

## 🧪 Запуск тестів (GoogleTest + CTest)

### 1. Запуск усіх тестів через CTest:
```bash
ctest --test-dir build --output-on-failure
```
або прямим запуском цілі тестів:
```bash
./build/unit_test
```

### 2. Запуск одного конкретного тесту або групи тестів:
```bash
# Запуск лише тестів розбору (Parser)
./build/unit_test --gtest_filter=ParserTest.*

# Запуск одного тесту регресії
./build/unit_test --gtest_filter=RegressionTest.MissionA_Hover
```

---

## 📋 Склад тестового набору

1. **Юніт-тести (Unit Tests)**:
   - `ParserTest`: розбір коректного сценарію, обробка порожнього сценарію, обробка пошкоджених даних.
   - `TrajectoryTest`: розрахунок точки скиду у балістичних моделях `HoverTrajectory` та `ForwardTrajectory`.
   - `FormatterTest`: точність форматування результатів (2 знаки після коми, правильний порядок полів).

2. **Регресійні тести (Regression Tests)**:
   - Перевірка сценаріїв ДЗ 5 (`mission-a` ... `mission-e`) у двох балістичних моделях (`hover` та `forward`) із фіксованими очікуваними координатами та боєприпасами.

---

## 🔒 Перевірка перед комітом (Git Hooks & Code Quality)

Всі правила форматування (`.clang-format`) та статичного аналізу (`.clang-tidy`) зафіксовані в репозиторії.

Щоб підключити автоматичну перевірку та запуск тестів перед кожним комітом, виконайте в терміналі:
```bash
git config core.hooksPath scripts
```



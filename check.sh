#!/bin/bash

# check.sh - Проверка форматирования кода с помощью clang-format
# Использование: ./check.sh [путь к директории или файлу]

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Проверяем наличие clang-format
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}Ошибка: clang-format не найден. Установите clang-format.${NC}"
    exit 1
fi

# Определяем путь для проверки
TARGET_PATH="${1:-.}"

# Проверяем, существует ли путь
if [[ ! -e "$TARGET_PATH" ]]; then
    echo -e "${RED}Ошибка: путь '$TARGET_PATH' не существует.${NC}"
    exit 1
fi

echo -e "${YELLOW}Проверка форматирования кода в: $TARGET_PATH${NC}"

# Находим все C++ файлы
if [[ -d "$TARGET_PATH" ]]; then
    FILES=$(find "$TARGET_PATH" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cxx" \) ! -path "*/build/*" ! -path "*/.git/*")
else
    FILES="$TARGET_PATH"
fi

# Проверяем, найдены ли файлы
if [[ -z "$FILES" ]]; then
    echo -e "${YELLOW}Файлы C++ не найдены в '$TARGET_PATH'.${NC}"
    exit 0
fi

echo "Найдено файлов для проверки: $(echo "$FILES" | wc -w)"

# Переменная для отслеживания ошибок
FAILED_FILES=()
TOTAL_FILES=0

# Проверяем каждый файл
while IFS= read -r file; do
    if [[ -n "$file" ]]; then
        TOTAL_FILES=$((TOTAL_FILES + 1))
        echo -n "Проверка $file... "
        
        # Проверяем форматирование
        if clang-format --dry-run -Werror "$file" > /dev/null 2>&1; then
            echo -e "${GREEN}OK${NC}"
        else
            echo -e "${RED}FAILED${NC}"
            FAILED_FILES+=("$file")
        fi
    fi
done <<< "$FILES"

# Выводим результаты
echo ""
echo "=========================================="
echo "Результаты проверки:"
echo "Всего файлов проверено: $TOTAL_FILES"
echo "Файлов с ошибками форматирования: ${#FAILED_FILES[@]}"

if [[ ${#FAILED_FILES[@]} -eq 0 ]]; then
    echo -e "${GREEN}✅ Все файлы правильно отформатированы!${NC}"
    exit 0
else
    echo -e "${RED}❌ Следующие файлы требуют форматирования:${NC}"
    for file in "${FAILED_FILES[@]}"; do
        echo -e "${RED}  - $file${NC}"
    done
    echo ""
    echo -e "${YELLOW}Для форматирования запустите: ./format.sh${NC}"
    exit 1
fi
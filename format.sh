#!/bin/bash

# format.sh - Форматирование кода с помощью clang-format
# Использование: ./format.sh [путь к директории или файлу]

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Проверяем наличие clang-format
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}Ошибка: clang-format не найден. Установите clang-format.${NC}"
    exit 1
fi

# Определяем путь для форматирования
TARGET_PATH="${1:-.}"

# Проверяем, существует ли путь
if [[ ! -e "$TARGET_PATH" ]]; then
    echo -e "${RED}Ошибка: путь '$TARGET_PATH' не существует.${NC}"
    exit 1
fi

echo -e "${BLUE}Форматирование кода в: $TARGET_PATH${NC}"

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

echo "Найдено файлов для форматирования: $(echo "$FILES" | wc -w)"

# Переменная для отслеживания обработанных файлов
FORMATTED_FILES=()
FAILED_FILES=()
UNCHANGED_FILES=()
TOTAL_FILES=0

# Форматируем каждый файл
while IFS= read -r file; do
    if [[ -n "$file" ]]; then
        TOTAL_FILES=$((TOTAL_FILES + 1))
        echo -n "Форматирование $file... "
        
        # Создаем временный файл для сравнения
        TEMP_FILE=$(mktemp)
        
        # Применяем clang-format
        if clang-format "$file" > "$TEMP_FILE"; then
            # Проверяем, изменился ли файл
            if ! cmp -s "$file" "$TEMP_FILE"; then
                # Файл изменился, перезаписываем
                cp "$TEMP_FILE" "$file"
                echo -e "${GREEN}FORMATTED${NC}"
                FORMATTED_FILES+=("$file")
            else
                # Файл не изменился
                echo -e "${BLUE}UNCHANGED${NC}"
                UNCHANGED_FILES+=("$file")
            fi
        else
            echo -e "${RED}FAILED${NC}"
            FAILED_FILES+=("$file")
        fi
        
        # Удаляем временный файл
        rm -f "$TEMP_FILE"
    fi
done <<< "$FILES"

# Выводим результаты
echo ""
echo "=========================================="
echo "Результаты форматирования:"
echo "Всего файлов обработано: $TOTAL_FILES"
echo "Файлов отформатировано: ${#FORMATTED_FILES[@]}"
echo "Файлов без изменений: ${#UNCHANGED_FILES[@]}"
echo "Файлов с ошибками: ${#FAILED_FILES[@]}"

if [[ ${#FORMATTED_FILES[@]} -gt 0 ]]; then
    echo -e "${GREEN}✅ Отформатированные файлы:${NC}"
    for file in "${FORMATTED_FILES[@]}"; do
        echo -e "${GREEN}  - $file${NC}"
    done
fi

if [[ ${#FAILED_FILES[@]} -gt 0 ]]; then
    echo -e "${RED}❌ Файлы с ошибками форматирования:${NC}"
    for file in "${FAILED_FILES[@]}"; do
        echo -e "${RED}  - $file${NC}"
    done
fi

if [[ ${#FAILED_FILES[@]} -eq 0 ]]; then
    echo -e "${GREEN}🎉 Форматирование завершено успешно!${NC}"
    exit 0
else
    exit 1
fi
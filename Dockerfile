FROM debian:12-slim

# Встановлюємо компілятор g++
RUN apt-get update && apt-get install -y g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копіюємо код програми та тести
COPY main.cpp .
COPY input/ ./input/

# 🌟 САМА СТВОРЮЄ ПРАВИЛЬНИЙ LINUX-СКРИПТ БЕЗ СИМВОЛІВ WINDOWS:
RUN printf '#!/bin/bash\nmkdir -p output\nfor input_file in input/*.txt; do\n  [ -e "$input_file" ] || continue\n  filename=$(basename "$input_file" .txt)\n  output_file="output/${filename}-result.txt"\n  echo "Processing: $input_file -> $output_file"\n  ./drop_calc --input-file "$input_file" --output-file "$output_file"\ndone\necho "Batch processing completed successfully!"\n' > run-all.sh

# Компілюємо С++ код
RUN g++ main.cpp -o drop_calc

# Даємо права на запуск скрипта
RUN chmod +x run-all.sh

CMD ["./run-all.sh"]

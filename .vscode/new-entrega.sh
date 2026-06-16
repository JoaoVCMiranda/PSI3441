#!/usr/bin/env bash
set -e

NAME="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO="$(dirname "$SCRIPT_DIR")"
TEMPLATE="$REPO/entregas/template"
TARGET="$REPO/entregas/$NAME"
WS="$SCRIPT_DIR/PSI3441.code-workspace"

if [ -z "$NAME" ]; then
    echo "Erro: informe o nome da entrega" >&2
    exit 1
fi

if [ -d "$TARGET" ]; then
    echo "Erro: $TARGET já existe" >&2
    exit 1
fi

cp -r "$TEMPLATE" "$TARGET"

# Substitui placeholders nos arquivos copiados
sed -i "s/Template — substitua pela descrição da entrega/Entrega $NAME/" "$TARGET/platformio.ini"
sed -i "s/PSI3441_template/PSI3441_$NAME/"                               "$TARGET/zephyr/CMakeLists.txt"
sed -i "s/Atividade N/Atividade $NAME/"                                   "$TARGET/relatorio.md"

# Adiciona ao workspace
python3 - <<PYEOF
import json
with open('$WS') as f:
    ws = json.load(f)
entry = {'path': '../entregas/$NAME'}
if entry not in ws['folders']:
    ws['folders'].append(entry)
with open('$WS', 'w') as f:
    json.dump(ws, f, indent='\t', ensure_ascii=False)
    f.write('\n')
PYEOF

echo "✓ entregas/$NAME criado e adicionado ao workspace"
echo "  Recarregue o workspace (Ctrl+Shift+P → 'Reload Window') para ver a nova pasta."

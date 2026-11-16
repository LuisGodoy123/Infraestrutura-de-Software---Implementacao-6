#!/bin/bash
cd "$(dirname "$0")" || exit 1
T=tests
pass=0
fail=0

make clean > /dev/null
if [ "$1" = "asan" ]; then
    export ASAN_OPTIONS=exitcode=99
    make CFLAGS="-Wall -Wextra -std=c99 -g -fsanitize=address" || exit 1
else
    make || exit 1
fi
BANKER=$(pwd)/banker
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT

check() {
    name=$1
    expected=$2
    cust=$3
    cmds=$4
    shift 4
    rm -rf "$W"/*
    [ "$cust" != "-" ] && cp "$T/$cust" "$W/customer.txt"
    [ "$cmds" != "-" ] && cp "$T/$cmds" "$W/commands.txt"
    (cd "$W" && "$BANKER" "$@" > stdout.txt 2> stderr.txt)
    code=$?
    ok=1
    [ "$code" -ne "$expected" ] && ok=0
    [ -s "$W/stdout.txt" ] && ok=0
    if [ "$expected" -eq 0 ]; then
        [ -e "$W/result.txt" ] || ok=0
        if [ -n "$EXPECTED" ]; then
            cmp -s "$EXPECTED" "$W/result.txt" || ok=0
        fi
    else
        [ -e "$W/result.txt" ] && ok=0
        [ -s "$W/stderr.txt" ] || ok=0
    fi
    grep -q Sanitizer "$W/stderr.txt" && ok=0
    if [ $ok -eq 1 ]; then
        pass=$((pass + 1))
        echo "PASS  $name"
    else
        fail=$((fail + 1))
        echo "FAIL  $name (saida $code, esperado $expected)"
    fi
}

echo "== exemplo do professor (result.txt byte a byte) =="
EXPECTED=$T/expected_result_ok.txt check "customer_ok + commands_ok" 0 customer_ok.txt commands_ok.txt 10 5 7
EXPECTED=$T/expected_result_ok.txt check "customer_ok_crlf + commands_ok_crlf" 0 customer_ok_crlf.txt commands_ok_crlf.txt 10 5 7

echo "== customer.txt (com commands_star_only.txt) =="
for c in customer_ok:0 customer_ok_crlf:0 customer_crlf:0 customer_blank_end:0 \
         customer_over_total:0 customer_empty:0 customer_only_blank:0 \
         customer_blank_mid:1 customer_few_fields:1 customer_many_fields:1 \
         customer_nonnumeric:1 customer_negative:1 customer_empty_field:1 \
         customer_overflow:1 customer_space:1; do
    check "${c%%:*}" "${c##*:}" "${c%%:*}.txt" commands_star_only.txt 10 5 7
done

echo "== commands.txt (com customer_ok.txt) =="
for c in commands_blank_end:0 commands_empty:0 commands_star_only:0 \
         commands_release_ok:0 commands_blank_mid:1 commands_unknown:1 \
         commands_lowercase:1 commands_few_fields:1 commands_many_fields:1 \
         commands_no_customer:1 commands_nonnumeric:1 commands_negative:1 \
         commands_overflow:1 commands_customer_out:1 commands_star_extra:1 \
         commands_double_space:1 commands_trailing_space:1 commands_star_space:1 \
         commands_release_too_much:1 commands_crlf:1; do
    check "${c%%:*}" "${c##*:}" customer_ok.txt "${c%%:*}.txt" 10 5 7
done

echo "== argumentos e arquivos ausentes =="
check "sem argumentos" 1 customer_ok.txt commands_ok.txt
check "menos argumentos que colunas (10 5)" 1 customer_ok.txt commands_ok.txt 10 5
check "mais argumentos que colunas (10 5 7 1)" 1 customer_ok.txt commands_ok.txt 10 5 7 1
check "argumento nao numerico (10 abc 7)" 1 customer_ok.txt commands_ok.txt 10 abc 7
check "argumento negativo (10 -5 7)" 1 customer_ok.txt commands_ok.txt 10 -5 7
check "sem customer.txt" 1 - commands_ok.txt 10 5 7
check "sem commands.txt" 1 customer_ok.txt - 10 5 7

echo
echo "$pass passaram, $fail falharam"
make clean > /dev/null
[ $fail -eq 0 ]

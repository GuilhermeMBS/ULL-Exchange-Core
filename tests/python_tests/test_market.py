import os
import csv
from scripts.generate_market import generateMarket

OUTPUT = "tests/python_tests/test_output.csv"

def cleanup():
    if os.path.exists(OUTPUT):
        os.remove(OUTPUT)

def test_retorna_zero_em_sucesso():
    result = generateMarket(OUTPUT, num_orders=10, failures=0.0)
    assert result == 0, f"Esperado 0, got {result}"
    cleanup()
    print("test_retorna_zero_em_sucesso: OK")

def test_retorna_erro_em_caminho_invalido():
    result = generateMarket("/caminho/invalido/x.csv", 10, 0.0)
    assert result == -4, f"Esperado -4, got {result}"
    print("test_retorna_erro_em_caminho_invalido: OK")

def test_arquivo_criado():
    generateMarket(OUTPUT, num_orders=10, failures=0.0)
    assert os.path.exists(OUTPUT), "Arquivo não foi criado"
    cleanup()
    print("test_arquivo_criado: OK")

def test_100_ordens_mistas():
    result = generateMarket(OUTPUT, num_orders=100, failures=0.1)
    assert result == 0, f"Esperado 0, got {result}"
    cleanup()
    print("test_100_ordens_mistas: OK")

def test_numero_de_linhas_correto():
    generateMarket(OUTPUT, num_orders=100, failures=0.1)
    with open(OUTPUT) as f:
        linhas = list(csv.reader(f))
    assert len(linhas) == 101, f"Esperado 101 linhas, got {len(linhas)}"
    cleanup()
    print("test_numero_de_linhas_correto: OK")

def test_timestamps_em_ordem_crescente():
    generateMarket(OUTPUT, num_orders=100, failures=0.0)
    with open(OUTPUT) as f:
        reader = csv.DictReader(f)
        timestamps = [row['timestamp'] for row in reader]
    assert timestamps == sorted(timestamps), "Timestamps fora de ordem"
    cleanup()
    print("test_timestamps_em_ordem_crescente: OK")

def test_side_apenas_C_ou_V():
    generateMarket(OUTPUT, num_orders=100, failures=0.0)
    with open(OUTPUT) as f:
        reader = csv.DictReader(f)
        for row in reader:
            assert row['side'] in ['C', 'V'], f"Side inválido: {row['side']}"
    cleanup()
    print("test_side_apenas_C_ou_V: OK")

def test_ordens_invalidas_tem_preco_ou_qty_invalidos():
    generateMarket(OUTPUT, num_orders=100, failures=1.0)
    with open(OUTPUT) as f:
        reader = csv.DictReader(f)
        for row in reader:
            price = float(row['price'])
            qty   = float(row['quantity'])
            assert price <= 0 or qty <= 0, "Ordem deveria ser inválida"
    cleanup()
    print("test_ordens_invalidas_tem_preco_ou_qty_invalidos: OK")

def test_ordens_validas_tem_preco_e_qty_positivos():
    generateMarket(OUTPUT, num_orders=100, failures=0.0)
    with open(OUTPUT) as f:
        reader = csv.DictReader(f)
        for row in reader:
            assert float(row['price']) > 0, "Preço deveria ser positivo"
            assert float(row['quantity']) > 0, "Quantidade deveria ser positiva"
    cleanup()
    print("test_ordens_validas_tem_preco_e_qty_positivos: OK")

def test_header_correto():
    generateMarket(OUTPUT, num_orders=10, failures=0.0)
    with open(OUTPUT) as f:
        header = csv.reader(f).__next__()
    assert header == ['id', 'timestamp', 'side', 'price', 'quantity'], f"Header errado: {header}"
    cleanup()
    print("test_header_correto: OK")

if __name__ == "__main__":
    test_retorna_zero_em_sucesso()
    test_retorna_erro_em_caminho_invalido()
    test_arquivo_criado()
    test_100_ordens_mistas()
    test_numero_de_linhas_correto()
    test_timestamps_em_ordem_crescente()
    test_side_apenas_C_ou_V()
    test_ordens_invalidas_tem_preco_ou_qty_invalidos()
    test_ordens_validas_tem_preco_e_qty_positivos()
    test_header_correto()
    print("\nTodos os testes passaram!")
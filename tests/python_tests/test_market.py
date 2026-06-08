import unittest
import os
import csv
from scripts.generate_market import generateMarket

class TestGenerateMarket(unittest.TestCase):

    def setUp(self):
        self.output = "tests/python_tests/test_output.csv"

    def tearDown(self):
        if os.path.exists(self.output):
            os.remove(self.output)

    # --- Testes básicos de retorno ---

    def test_retorna_zero_em_sucesso(self):
        """Deve retornar 0 (sucesso absoluto) em geração normal"""
        result = generateMarket(self.output, num_orders=10, failures=0.0)
        self.assertEqual(result, 0)

    def test_retorna_erro_em_caminho_invalido(self):
        """Deve retornar -4 se não conseguir criar o arquivo"""
        result = generateMarket("/caminho/invalido/x.csv", 10, 0.0)
        self.assertEqual(result, -4)

    def test_arquivo_criado(self):
        """Arquivo CSV deve existir após geração"""
        generateMarket(self.output, num_orders=10, failures=0.0)
        self.assertTrue(os.path.exists(self.output))

    # --- Caso de Teste do Documento ---
    # Gerar mercado com 100 ordens misturadas (válidas, inválidas,
    # compras parciais, filas e critérios de desempate por tempo)
    # ordenadas em tempo crescente

    def test_100_ordens_mistas(self):
        """100 ordens com mistura de válidas e inválidas — deve retornar 0"""
        result = generateMarket(self.output, num_orders=100, failures=0.1)
        self.assertEqual(result, 0)

    def test_numero_de_linhas_correto(self):
        """CSV deve ter exatamente 100 ordens + 1 linha de header"""
        generateMarket(self.output, num_orders=100, failures=0.1)
        with open(self.output) as f:
            linhas = list(csv.reader(f))
        self.assertEqual(len(linhas), 101)

    def test_timestamps_em_ordem_crescente(self):
        """Timestamps devem estar ordenados em ordem crescente"""
        generateMarket(self.output, num_orders=100, failures=0.0)
        with open(self.output) as f:
            reader = csv.DictReader(f)
            timestamps = [row['timestamp'] for row in reader]
        self.assertEqual(timestamps, sorted(timestamps))

    def test_side_apenas_C_ou_V(self):
        """Todas as ordens válidas devem ter side 'C' ou 'V'"""
        generateMarket(self.output, num_orders=100, failures=0.0)
        with open(self.output) as f:
            reader = csv.DictReader(f)
            for row in reader:
                self.assertIn(row['side'], ['C', 'V'])

    def test_ordens_invalidas_tem_preco_ou_qty_invalidos(self):
        """Com failures=1.0, todas as ordens devem ter preço ou qty inválidos"""
        generateMarket(self.output, num_orders=100, failures=1.0)
        with open(self.output) as f:
            reader = csv.DictReader(f)
            for row in reader:
                price = float(row['price'])
                qty   = float(row['quantity'])
                self.assertTrue(price <= 0 or qty <= 0)

    def test_ordens_validas_tem_preco_e_qty_positivos(self):
        """Com failures=0.0, todas as ordens devem ter preço e qty positivos"""
        generateMarket(self.output, num_orders=100, failures=0.0)
        with open(self.output) as f:
            reader = csv.DictReader(f)
            for row in reader:
                self.assertGreater(float(row['price']), 0)
                self.assertGreater(float(row['quantity']), 0)

    def test_header_correto(self):
        """CSV deve ter exatamente as colunas esperadas"""
        generateMarket(self.output, num_orders=10, failures=0.0)
        with open(self.output) as f:
            header = csv.reader(f).__next__()
        self.assertEqual(header, ['id', 'timestamp', 'side', 'price', 'quantity'])

if __name__ == "__main__":
    unittest.main()
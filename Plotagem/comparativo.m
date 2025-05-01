% (1) Compilar o programa em C
system('gcc Fila_Heap_2.c -o Fila_Heap');

% (2) Executar o programa
system('./Fila_Heap');

% (3) Agora o programa já criou os arquivos .txt

% Ler arquivos
insercao = readmatrix('dados_insercao.txt');
remocao = readmatrix('dados_remocao.txt');

% Separar inserção
valores_insercao = insercao(:, 1);
comparacoes_fila_insercao = insercao(:, 2);
comparacoes_heap_insercao = insercao(:, 3);

% Separar remoção
valores_remocao = remocao(:, 1);
comparacoes_fila_remocao = remocao(:, 2);
comparacoes_heap_remocao = remocao(:, 3);

% ===============================
% FIGURA 1: INSERÇÃO
% ===============================

figure('Position', [100, 100, 800, 600]);
h1 = scatter(valores_insercao, comparacoes_fila_insercao, 30, '.', 'MarkerEdgeColor', 'b', 'DisplayName', 'Fila');
hold on;
h2 = scatter(valores_insercao, comparacoes_heap_insercao, 30, '.', 'MarkerEdgeColor', 'r', 'DisplayName', 'Heap');
hold off;

title('Inserção', 'FontSize', 15, 'FontWeight', 'bold');
xlabel('Valor Inserido', 'FontSize', 10, 'FontAngle', 'italic');
ylabel('Comparações', 'FontSize', 12, 'FontAngle', 'italic');
legend('show', 'Location', 'northeast');
grid on;
set(gca, 'GridLineStyle', ':', 'FontSize', 12);

% Salvar
saveas(gcf, 'grafico_insercao.png');

% ===============================
% FIGURA 2: REMOÇÃO
% ===============================

figure('Position', [100, 100, 800, 600]);
h3 = scatter(valores_remocao, comparacoes_fila_remocao, 30, '.', 'MarkerEdgeColor', 'b', 'DisplayName', 'Fila');
hold on;
h4 = scatter(valores_remocao, comparacoes_heap_remocao, 30, '.', 'MarkerEdgeColor', 'r', 'DisplayName', 'Heap');
hold off;

title('Remoção', 'FontSize', 15, 'FontWeight', 'bold');
xlabel('Valor Removido', 'FontSize', 10, 'FontAngle', 'italic');
ylabel('Comparações', 'FontSize', 12, 'FontAngle', 'italic');
legend('show', 'Location', 'northeast');
grid on;
set(gca, 'GridLineStyle', ':', 'FontSize', 12);

% Salvar
saveas(gcf, 'grafico_remocao.png');

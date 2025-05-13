% === COMPILAR E EXECUTAR O PROGRAMA C === 
system('gcc Fila_Heap_3.c -o Fila_Heap -lm');
system('./Fila_Heap');

% === LER DADOS ===
remocao = readmatrix('dados_remocao.txt');

% === SEPARAR COLUNAS ===
valores_remocao = remocao(:, 1);
comparacoes_fila_remocao = remocao(:, 2);
comparacoes_heap_remocao = remocao(:, 3);

% === CONFIGURAÇÕES DE FIGURA ===
set(0, 'DefaultFigureWindowStyle', 'docked');  % Janelas de figura lado a lado
smooth_factor = 15;  % Janela para suavização Gaussian

% === APLICA SUAVIZAÇÃO ===
smoothed_fila_rem = smoothdata(comparacoes_fila_remocao, 'gaussian', smooth_factor);
smoothed_heap_rem = smoothdata(comparacoes_heap_remocao, 'gaussian', smooth_factor);

% === PLOT DO GRÁFICO DE REMOÇÃO ===
figure(2);
clf; % <-- Limpa o conteúdo da figura 2 antes de desenhar
plot(valores_remocao, smoothed_fila_rem, 'b-', 'LineWidth', 2); hold on;
plot(valores_remocao, smoothed_heap_rem, 'r-', 'LineWidth', 2);
title('Remoção (Suavizado)');
xlabel('Valor Removido');
ylabel('Número de Comparações');
legend('Fila', 'Heap', 'Location', 'northwest');
grid on;

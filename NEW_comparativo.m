% === COMPILAR E EXECUTAR O PROGRAMA C ===
system('gcc Fila_Heap_2.c -o Fila_Heap -lm');
system('./Fila_Heap');

% === LER DADOS ===
insercao = readmatrix('dados_insercao.txt');
remocao  = readmatrix('dados_remocao.txt');

% === SEPARAR COLUNAS ===
valores_insercao            = insercao(:, 1);
comparacoes_fila_insercao   = insercao(:, 2);
comparacoes_heap_insercao   = insercao(:, 3);

valores_remocao             = remocao(:, 1);
comparacoes_fila_remocao    = remocao(:, 2);
comparacoes_heap_remocao    = remocao(:, 3);

% === FIGURA 1: INSERÇÃO ===
figure('Name','Inserção','Position',[100 100 800 600]);
smoothed_fila_insercao = movmean(comparacoes_fila_insercao, 5);
smoothed_heap_insercao = movmean(comparacoes_heap_insercao, 5);

plot(valores_insercao, smoothed_fila_insercao, 'b-', 'LineWidth', 2); hold on;
plot(valores_insercao, smoothed_heap_insercao, 'r-', 'LineWidth', 2);

title('Inserção','FontSize',15);
xlabel('Valor Inserido','FontSize',12,'FontAngle','italic');
ylabel('Comparações','FontSize',12,'FontAngle','italic');
legend('Fila','Heap','Location','northwest');
grid on;


% === FIGURA 2: REMOÇÃO ===
figure('Name','Remoção','Position',[100 100 800 600]);

% Suavizar curvas com média móvel
smoothed_fila = movmean(comparacoes_fila_remocao, 5);
smoothed_heap = movmean(comparacoes_heap_remocao, 5);

plot(valores_remocao, smoothed_fila, 'b-', 'LineWidth', 2); hold on;
plot(valores_remocao, smoothed_heap, 'r-', 'LineWidth', 2);

title('Remoção','FontSize',15);
xlabel('Valor Removido','FontSize',12,'FontAngle','italic');
ylabel('Comparações','FontSize',12,'FontAngle','italic');
legend('Fila','Heap','Location','northwest');
grid on;

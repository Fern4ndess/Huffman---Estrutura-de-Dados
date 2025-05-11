% === COMPILAR E EXECUTAR O PROGRAMA C === 
disp('Compilando programa C...');
status = system('gcc Fila_Heap_2.c -o Fila_Heap -lm');
if status ~= 0
    error('Falha na compilação');
end

disp('Executando programa C...');
status = system('./Fila_Heap');
if status ~= 0
    error('O programa C falhou ao executar');
end

% === LER DADOS ===
disp('Lendo arquivos de dados...');
try
    insercao = readmatrix('dados_insercao.txt');
    remocao = readmatrix('dados_remocao.txt');
catch
    error('Erro ao ler arquivos de dados');
end

if isempty(insercao) || isempty(remocao)
    error('Arquivos de dados estão vazios');
end

% === SEPARAR COLUNAS ===
valores_insercao = insercao(:, 1);
comparacoes_fila_insercao = insercao(:, 2);
comparacoes_heap_insercao = insercao(:, 3);

valores_remocao = remocao(:, 1);
comparacoes_fila_remocao = remocao(:, 2);
comparacoes_heap_remocao = remocao(:, 3);

% === ANÁLISE DOS DADOS ANTES DE PLOTAR ===
% Adicione esta nova seção para análise dos dados
disp('Analisando dados de remoção...');
disp(['Mínimo de comparações (Fila): ' num2str(min(comparacoes_fila_remocao))]);
disp(['Máximo de comparações (Fila): ' num2str(max(comparacoes_fila_remocao))]);
disp(['Mínimo de comparações (Heap): ' num2str(min(comparacoes_heap_remocao))]);
disp(['Máximo de comparações (Heap): ' num2str(max(comparacoes_heap_remocao))]);

% Filtrar valores inválidos (substitua 1000 pelo valor máximo esperado)
limite_superior = 1000;
validos = comparacoes_fila_remocao >= 0 & comparacoes_fila_remocao <= limite_superior & ...
          comparacoes_heap_remocao >= 0 & comparacoes_heap_remocao <= limite_superior;

valores_remocao_validos = valores_remocao(validos);
comparacoes_fila_validas = comparacoes_fila_remocao(validos);
comparacoes_heap_validas = comparacoes_heap_remocao(validos);

% === FIGURAS ===
try
    % Configuração para manter as figuras no MATLAB
    set(0, 'DefaultFigureWindowStyle', 'docked');
    
    % Suavização
    smooth_factor = 15;  % Aumentei para melhor suavização
    
    % Figura 1 - Inserção (mantido como estava)
    figure(1);
    smoothed_fila_ins = smoothdata(comparacoes_fila_insercao, 'rloess', smooth_factor);
    smoothed_heap_ins = smoothdata(comparacoes_heap_insercao, 'rloess', smooth_factor);
    
    plot(valores_insercao, smoothed_fila_ins, 'b-', 'LineWidth', 2); hold on;
    plot(valores_insercao, smoothed_heap_ins, 'r-', 'LineWidth', 2);
    title('Inserção - Comparações Fila vs Heap (suavizado)');
    xlabel('Valor Inserido');
    ylabel('Número de Comparações');
    legend('Fila', 'Heap', 'Location', 'northwest');
    grid on;
    hold off;
    
    % Figura 2 - Remoção (MODIFICADA para usar dados filtrados)
    figure(2);
    smoothed_fila_rem = smoothdata(comparacoes_fila_validas, 'gaussian', smooth_factor);
    smoothed_heap_rem = smoothdata(comparacoes_heap_validas, 'gaussian', smooth_factor);
    
    plot(valores_remocao_validos, smoothed_fila_rem, 'b-', 'LineWidth', 2); hold on;
    plot(valores_remocao_validos, smoothed_heap_rem, 'r-', 'LineWidth', 2);
    title('Remoção - Comparações Validas (suavizado)');
    xlabel('Valor Removido');
    ylabel('Número de Comparações');
    legend('Fila', 'Heap', 'Location', 'northwest');
    grid on;
    
    % Ajustar automaticamente o limite do eixo Y
    ylim([0, max([smoothed_fila_rem; smoothed_heap_rem]) * 1.1]);
    hold off;
    
catch e
    error(['Erro ao criar figuras: ' e.message]);
end

disp('Script concluído. Figuras devem estar dockadas no MATLAB.');
disp(['Dados válidos de remoção: ' num2str(sum(validos)) ' de ' num2str(length(validos))]);

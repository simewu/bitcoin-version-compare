data = readmatrix('logDirectoryOutput.csv');
data_str = readtable('logDirectoryOutput.csv');

fontSize = 16;

f = figure;


x = table2array(data_str(2:end, 1))
y = data(2:end, 17);
y_B = data(2:end, 19);

hold on;
bar(y_B, 'FaceColor', '#FFF');
bar(y, 'FaceColor', '#444');
xticks(1:length(x))
xticklabels(x)

xlabel('Bitcoin Core Version')
ylabel('Percentage Difference')
xtickangle(45);

%axis square;
ylim([0, 1])
set(gca, 'YGrid', 'on', 'YMinorGrid', 'on');
%set(gca, 'XScale', 'log', 'XTick',x_avg, 'XTickLabel', x_avg, 'YGrid', 'on', 'YMinorGrid', 'on');
%set(gca, 'XTick',x_avg, 'XTickLabel', x_avg_real, 'YGrid', 'on', 'YMinorGrid', 'on');
set(gca,'FontSize', fontSize);

legend('Bytes of code', 'Number of code files', 'Location', 'NorthWest', 'NumColumns', 2)
% ax = gca
% ax.XAxis.FontSize = fontSize;
% ax.YAxis.FontSize = fontSize;

f.Position = [100 100 1500 400];
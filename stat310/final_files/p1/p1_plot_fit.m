function p1_plot_fit(b)
    [fit, res, data] = ensoFitRes(b);
    plot(1:168, fit);
    hold on;
    scatter(1:168, data);
    hold off;
    
end


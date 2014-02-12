function [iters, a] = p1()
    sizes = [100, 200, 300, 400, 500, 650, 800, 1000, 1330, 1660, 2000];
    iters = zeros(1,length(sizes));
    a = zeros(1,length(sizes));
    
    for i=1:length(sizes)
        sizes(i)
        for j=1:20
            [~, iters_t, a_t] = optimize_bt(@rosenbrock, randn(sizes(i), 1), 0.1, 0.5, 0.1, 10e-6);
            iters(i) = iters(i) + iters_t/20;
            a(i) = a(i) + a_t/20;
        end
    end
end


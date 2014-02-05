function [iters, fevals] = p2(sizes)
    [~, n] = size(sizes);
    iters = 1:n;
    fevals = 1:n;
    
    for i = 1:n
        [~, iters(i), fevals(i)] = optimize(@cute_wrap, ones(sizes(i), 1), 0.1, 0.5, 0.1, 0.00000001);
    end
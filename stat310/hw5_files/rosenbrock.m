function [f, g, H] = rosenbrock(x, n)
    if n >= 0
        f = rosen_fn(x);
    end
    if n >= 1
        g = rosen_gr(x);
    end
    if n >= 2
        H = rosen_H(x);
    end
end
        
function y = rosen_fn(x)    
    N = length(x) / 2;
    I = 1:N;
    y = sum((1 - x(2*I-1)).^2) + 10*sum((x(2*I) - x(2*I-1).^2).^2);
end

function g = rosen_gr(x)
    N = length(x) / 2;
    g = 1:2*N;
    for i=1:N
        g(2*i-1) = -2*(1-x(2*i-1))-40*x(2*i-1)*(x(2*i)-x(2*i-1)^2);
        g(2*i) = 20*(x(2*i)-x(2*i-1)^2);
    end
    g = g.';
end

function H = rosen_H(x)
    N = length(x) / 2;
    row_inds = 1:4*N;
    col_inds = 1:4*N;
    data = 1:4*N;
    for i = 1:N
        row_inds(4*i-3) = 2*i;
        col_inds(4*i-3) = 2*i;
        data(4*i-3) = 20;

        row_inds(4*i-2) = 2*i-1;
        col_inds(4*i-2) = 2*i;
        data(4*i-2) = -40*x(2*i-1);

        row_inds(4*i-1) = 2*i;
        col_inds(4*i-1) = 2*i-1;
        data(4*i-1) = -40*x(2*i-1);

        row_inds(4*i) = 2*i-1;
        col_inds(4*i) = 2*i-1;
        data(4*i) = 2 - 40*x(2*i)+120*x(2*i-1)^2;
    end
    H = sparse(row_inds, col_inds, data, 2*N, 2*N);
end

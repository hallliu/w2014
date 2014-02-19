function [fx, gx] = matern_fn(X, y, theta)
    [~, n] = size(X);
    d = length(theta);
        
    K = matern_matrix(X, theta);
    L = chol(K, 'lower');
        
    Kderivs = cell([1, d]);
    tr_kkderiv = 1:d;
    for i = 1:d
        Kderivs{i} = matern_deriv(K, X, theta, i);
        tr_kkderiv(i) = trace(L.'\(L\Kderivs{i}));
    end
    Kdet = prod(diag(L))^2;

    
    kinv_y = L.'\(L\y);
    fx = 0.5 * y.' * kinv_y + 0.5 * log(Kdet) + 0.5 * n * log(2*pi);
    
    gx = (1:d).';
    for i = 1:d
        gx(i) = -0.5 * (kinv_y.' * Kderivs{i} * kinv_y) + 0.5 * tr_kkderiv(i);
    end
    
end

function K = calculate_matern_matrix(X, n, theta)
    K = zeros(n, n);
    for i = 1:n
        for j = 1:i
            K(i, j) = calculate_matern_entry(X(:, i), X(:, j), theta);
            K(j, i) = K(i, j);
        end
    end
end

function k = calculate_matern_entry(x, y, theta)
    r = sqrt(sum(((x - y)./theta).^2));
    k = (1 + sqrt(5)*r+5*r^2/3)*exp(-sqrt(5)*r);
end

function dK = calculate_kderiv(K, X, theta, l)
    [n, ~] = size(K);
    dK = zeros(n, n);
    for i = 1:n
        for j = 1:i
            mijl = 5 * (X(l, i) - X(l, j))^2 / (3 * theta(l)^3);
            rij = sqrt(sum(((X(:, i) - X(:, j))./theta).^2));
            dK(i, j) = (K(i, j) - (5/3)*rij^2*exp(-sqrt(5)*rij)) * mijl;
            dK(j, i) = dK(i, j);
        end
    end
end

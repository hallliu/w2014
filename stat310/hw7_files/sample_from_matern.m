function S = sample_from_matern(X, theta, r)
    [~, n] = size(X);
    K = matern_matrix(X, theta);
    L = chol(K);
    S = L * randn(n, r);
end

function K = calculate_matern_matrix(X, theta)
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
    k = (1 + sqrt(5)*r+(5/3)*r^2)*exp(-sqrt(5)*r);
end
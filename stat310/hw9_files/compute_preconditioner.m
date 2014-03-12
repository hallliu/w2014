function f = compute_preconditioner(B)
    [L, D, ~] = ldl(B, 'vector');
    % Loop through and compute the proper lambda matrix
    [n, ~] = size(D);
    Lambda = D;
    i = 1;
    while i <= n
        if (i == n || D(i, i+1) == 0)
            if D(i, i) > 1e-5
                Lambda(i, i) = sqrt(D(i, i));
            else
                Lambda(i, i) = 1;
            end
            i = i + 1;
            continue;
        end
        if (D(i, i) <= 1e-5)
            Lambda(i:i+1, i:i+1) = eye(2);
        elseif D(i+1, i+1) < D(i, i+1)^2/D(i, i) + 1e-5
            Lambda(i:i+1, i+i+1) = eye(2);
        else
            Lambda(i, i) = sqrt(D(i, i));
            Lambda(i+1, i) = D(i+1, i) / sqrt(D(i, i));
            Lambda(i, i+1) = 0;
            Lambda(i+1, i+1) = sqrt(D(i+1, i+1) - D(i, i+1)^2/D(i, i));
        end
        i = i + 2;
    end
    f = @(x)(compute_mx(L, Lambda, x));
end


% This function used to build the closure
function y = compute_mx (L, Lambda, x)
    y = Lambda.' \ x;
    y = L.' \ y;
    y = L \ y;
    y = Lambda \ y;
end

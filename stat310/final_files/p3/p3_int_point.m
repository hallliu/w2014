% Parameters: 
% start is some arbitrary starting point, specifying x, y, and lambda
% epsilon is the stopping tolerance
% M and c are the data matrix and label vector
 
function x = p3_int_point(start, epsilon, M, c)
    % First compute the necessary matrices
    [n, m] = size(M);
    G = diag([ones(m) 0]);
    A = [bsxfun(@times, c, M) ones(n, 1)];
    
    x = start(1:m+1);
    y = start(m+2:m+n+1);
    lambda = start(m+n+2:m+2*n+1);
    
    % Do the recommended correction to the starting point
    [dxaff, dyaff, dlaff] = compute_affine_scaling(G, A, x, y, lambda);
    y = max(1, abs(y + dyaff));
    lambda = max(1, abs(lambda+dlaff));
    
    % Start looping
    while 1
        grad_l = G*x - A.' * lambda;
        if norm(grad_l) < epsilon
            break;
        end
        [dxaff, dyaff, dlaff] = compute_affine_scaling(G, A, x, y, lambda);
        mu = y.' * lambda / n;
        aaff = get_maxmult(y, lambda, dyaff, dlaff);
        muaff = (y + aaff*dyaff).' * (lambda + aaff*dlaff) / n;
        sigma = (muaff / mu)^3;
        [dx, dy, dl] = compute_step(G, A, x, y, lambda, sigma, mu);
        tau = 1/(1+exp(-1/norm(grad_l)));
        alpha = get_maxmult(tau * y, tau * lambda, dy, dl);
        x = x + alpha * dx;
        y = y + alpha * dy;
        lambda = lambda + alpha * dl;
    end
end

function alpha = get_maxmult(a, b, da, db)
    asum = a + da;
    bsum = b + db;
    [amin, ai] = min(asum);
    [bmin, bi] = min(bsum);
    minval = min(amin, bmin);
    if asum(ai) == minval
        alpha = minval - a(ai) / da(ai);
    else
        alpha = minval - b(bi) / db(bi);
    end
end
function [dxaff, dyaff, dlaff] = compute_affine_scaling(G, A, x, y, lambda)
    % Construct the matrix we want to solve with
    Linvy = y ./ lambda;
    X = [G A.';A -diag(Linvy)];
    % Construct the RHS
    v1 = -(G*x - A.'*lambda);
    v2 = -A*x + ones(length(lambda), 1);
    v = [v1; v2];
    % Do the solve
    result = X \ v;
    % Extract delta x and delta lambda
    dxaff = result(1:length(x));
    dlaff = -result(length(x)+1:length(result));
    % Compute delta y
    dyaff = A * dxaff + A*x - y - ones(length(y));
end
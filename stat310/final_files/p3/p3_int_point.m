% Parameters: 
% start is some arbitrary starting point, specifying x, y, and lambda
% epsilon is the stopping tolerance
% M and c are the data matrix and label vector
 
function x = p3_int_point(start, epsilon, M, c)
    % First compute the necessary matrices
    [n, m] = size(M);
    G = eye(m+1);
    G(m+1, m+1) = 0;
    A = [bsxfun(@times, c, M) c];
    
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
        norm(grad_l)
        if norm(grad_l) < epsilon && norm(A*x - y - ones(n, 1)) < epsilon && norm(y .* lambda) < epsilon
            break;
        end
        [dxaff, dyaff, dlaff] = compute_affine_scaling(G, A, x, y, lambda);
        mu = y.' * lambda / n;
        aaff = get_maxmult([y; lambda], [dyaff; dlaff]);
        muaff = (y + aaff*dyaff).' * (lambda + aaff*dlaff) / n;
        sigma = (muaff / mu)^3;
        [dx, dy, dl] = compute_step(G, A, x, y, lambda, dyaff, dlaff, sigma, mu);
   
        tau = 0.2;%1/(1+exp(-0.001/norm(grad_l)));
        alpha = get_maxmult([tau * y; tau * lambda], [dy; dl]);
        x = x + alpha * dx;
        y = y + alpha * dy;
        lambda = lambda + alpha * dl;
    end
end

% Finds greatest alpha such that a-alpha*da>=0
function alpha = get_maxmult(a, da)
    rs = -a ./ da;
    rs(rs <= 0) = 1;
    alpha = min(rs);
end

function [dxaff, dyaff, dlaff] = compute_affine_scaling(G, A, x, y, lambda)
    [n, m] = size(A);
    M2 = bsxfun(@times, lambda./y, A);
    M2 = A.' * M2;
    X = G + M2;
    
    v1 = -G * x + A.' * lambda - A.' * ((lambda .* (A*x)) ./ y) + A.' * (lambda ./ y);
    dxaff = X \ v1;
    dlaff = lambda .* (-A * (x + dxaff) + ones(n, 1)) ./ y;
    dyaff = A * dxaff + A*x - y - ones(n, 1);
end

function [dx, dy, dl] = compute_step(G, A, x, y, lambda, dyaff, dlaff, sigma, mu)
    M2 = bsxfun(@times, lambda./y, A);
    M2 = A.' * M2;
    X = G + M2;

    v1 = -G * x + A.' * lambda - A.' * ((dlaff .* dyaff) ./ y) + sigma*mu*(A.' * (1./y));
    v1 = v1 - A.' * ((lambda .* (A*x)) ./ y) + A.' * (lambda ./ y);
    
    dx = X \ v1;
    dl = -lambda .* (A * (x+dx)) ./ y - ((dlaff .* dyaff) ./ y) + (sigma * mu ./ y) + lambda ./ y;
    dy = A * dx + A*x - y - ones(length(y), 1);

end

%{
function [dxaff, dyaff, dlaff] = compute_affine_scaling1(G, A, x, y, lambda)
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
    dyaff = A * dxaff + A*x - y - ones(length(y), 1);
end

function [dx, dy, dl] = compute_step1(G, A, x, y, lambda, dyaff, dlaff, sigma, mu)
    % Construct the matrix we want to solve with
    Linvy = y ./ lambda;
    X = [G A.';A -diag(Linvy)];
    % Construct the RHS
    v1 = -(G*x - A.'*lambda);
    v2 = -A*x + ones(length(lambda), 1) + sigma*mu*1./lambda - (dyaff .* dlaff) ./ lambda;
    v = [v1; v2];
    % Do the solve
    result = X \ v;
    % Extract delta x and delta lambda
    dx = result(1:length(x));
    dl = -result(length(x)+1:length(result));
    % Compute delta y
    dy = A * dx + A*x - y - ones(length(y), 1);
    %{
    [n, m] = size(A);
    K = [G zeros(m, n) -A.';A -eye(n) zeros(n); zeros(n, m) diag(lambda) diag(y)];
    vc = [dx;dy;dl];
    rc = G*x - A.'*lambda;
    rp = A*x-y-ones(n, 1);
    rl = -lambda.*y - dlaff.*dyaff + sigma*mu*ones(n, 1);
    bvec = [-rc;-rp;rl];
    norm(K*vc-bvec)
    %}

end
%}
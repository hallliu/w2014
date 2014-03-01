function [t1, t2, t3] = p1()
    sizes = [400, 600, 1000, 3000, 5000, 10000];
    k = length(sizes);
    t1 = zeros(1,k); t2 = zeros(1,k); t3 = zeros(1,k);
    nreps = 3;
    for r=1:k
        makelap(sizes(r));
        for i=1:nreps
            tic;
            cg_no_pre(sizes(r));
            t1(r) = t1(r) + toc/nreps;
            
            tic;
            cg_pre_L(sizes(r));
            t2(r) = t2(r) + toc/nreps;
            
            tic;
            chol_solv(sizes(r));
            t3(r) = t3(r) + toc/nreps;
        end
    end
end

function x = chol_solv(n)
    b = -ones(n, 1);
    L = makelap(n);
    w = sin((1:n).'/n*pi);
    C = chol(L, 'lower');
    % Use Sherman-Morrison formula to compute (L+ww^T)^{-1}b
    cb = C.'\(C\b);
    cw = C.'\(C\w);
    x = cb -  (w.'*cb) * cw / (1 + w.'*cw);
end

function x = cg_pre_L(n)
    b = -ones(n, 1);
    L = makelap(n);
    x = pcg(@(x)(qfun(x, n)), b, 1e-7, n+1, L);
end

function x = cg_no_pre(n)
    b = -ones(n, 1);
    x = pcg(@(x)(qfun(x, n)), b, 1e-7, 2*n+1);
end

function qx = qfun(x, n)
    persistent N w;
    if isempty(N) || N ~= n
        w = sin((1:n).'/n*pi);
        N = n;
    end
    L = makelap(n);
    qx = L*x + w * (w.' * x);
end

function lap = makelap(n)
    persistent L N;
    if isempty(N) || N ~= n
        N = n;
        lap_tmp=2*diag(ones(N,1))-diag(ones(N-1,1),1)-diag(ones(N-1,1),-1);
        [i,j,s] = find(lap_tmp);
        L = sparse(i, j, s, N, N);
    end
    lap = L;
end
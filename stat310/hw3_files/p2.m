function x = p2(upto)
    sizes = 25*2.^(1:upto);
    sp_times = 1:upto;
    d_times = 1:upto;
    
    for i=1:upto
        W_sp = makeW(sizes(i), 1);
        W_d = makeW(sizes(i), 0);
        e = ones(sizes(i)*2, 1);
        
        tic;
        ldlsolve(W_d, e);
        d_times(i) = toc;
        
        tic;
        x = ldlsolve(W_sp, e);
        sp_times(i) = toc;
        figure;
        plot(x);
    end
    
    d_coefs = polyfit(log(sizes), log(d_times), 1);
    s_coefs = polyfit(log(sizes), log(sp_times), 1);
    sprintf('Dense: C=%.3g, alpha=%.3f\n', exp(d_coefs(2)), d_coefs(1))
    sprintf('Sparse: C=%.3g, alpha=%.3f\n', exp(s_coefs(2)), s_coefs(1))
end

function x = ldlsolve(A, b)
    [L,D,p] = ldl(A, 'vector');
    b = b(p);
    
    dltx = L\b;
    if issparse(A)
        ltx = D\dltx;
    else
        ltx = blk2_slv(D, dltx).';
    end
    
    x = L.'\ltx;
    
    x(p) = x;
end

% This assumes a 2x2 block matrix in D and solves Dx=b. Appears to be
% faster than doing D\b for dense D.
function x = blk2_slv(D, b)
    [n, ~] = size(D);
    x = 1:n;
    i = 1;
    while i <= n
        if i==n
            x(i) = b(i) / D(i,i); i = i + 1;
            continue;
        end
            
        if D(i,i+1) ~= 0
            x(i:i+1) = D(i:i+1, i:i+1)\b(i:i+1); i = i + 2;
        else
            x(i) = b(i) / D(i, i); i = i + 1;
        end
    end
end

function W = makeW(N, sp)
    diag_entries = [ones(1, N), zeros(1, N)];
    lap_tmp=2*diag(diag_entries)-diag(ones(2*N-1,1),1)-diag(ones(2*N-1,1),-1);
    if sp
        [i,j,s] = find(lap_tmp);
        W = sparse(i, j, s, 2*N, 2*N);
    else
        W = lap_tmp;
    end
end

function x = p2(A, b)
    x = ldlsolve(A, b);
end

function x = ldlsolve(A, b)
    [L,D,p] = ldl(A, 'vector');
    b_permut = b(p);
    opts.LT = true;
    
    dltx = linsolve(L, b_permut, l_opts);
    ltx = blk2_slv(D, dltx);
    opts.TRANSA = true;
    x = linsolve(L, ltx, opts);
    
    x(p) = x;
end

%{
Solves a 2x2 block diagonal system
%}
function x = blk2_slv(D, b)
    [n, ~] = size(D);
    x = 1:n;
    i = 1;
    while i <= n
        if i==n
            x(i) = b(i) / D(i,i);
            i = i + 1;
            continue;
        end
            
        if D(i,i+1) ~= 0
            x(i:i+1) = D(i:i+1, i:i+1)\b(i:i+1);
            i = i + 2;
        else
            x(i) = b(i) / D(i, i);
            i = i + 1;
        end
    end
end
            
            
            
            

function [x, iters, fevals] = optimize(f, start, delta, rho, c_val, epsilon)
    x = start;
    old_fx = NaN;
    iters = 0;
    fevals = 0;
    
    while 1
        iters = iters + 1;
        [fx, gx, Hx] = f(x.', 2);
        fevals = fevals + 1;
        if ~isnan(old_fx) && abs(fx - old_fx) < epsilon
            break
        end
        [L, D, p] = modified_ldl(Hx, delta);
        pk = ldlsolve(L, D, p, -gx);
        %Backtracking part -- too much trouble to factor out, too many args
        ak = 1;
        f_val = f(x + ak * pk, 0);
        fevals = fevals + 1;
        while f_val > fx + c_val * ak * (gx.'*pk)
            ak = rho * ak;
            f_val = f(x + ak * pk, 0);
            fevals = fevals + 1;
        end
        
        old_fx = fx;
        x = x + ak * pk;
    end
end
        
function x = ldlsolve(L, D, p, b)
    b = b(p);
    
    dltx = L\b;
    ltx = D\dltx;
    x = L.'\ltx;
    
    x(p) = x;
end


function [L, D, p] = modified_ldl(A, delta)
    [L, D, p] = ldl(A, 'vector');
    [n, ~] = size(D);
    i = 1;
    while i <= n
        if (i == n || D(i, i+1) == 0)
            if D(i, i) < delta
                D(i, i) = delta;
            end
            i = i + 1;
            continue
        end
        a = D(i, i);
        b = D(i+1, i+1);
        c = D(i, i+1);
        sr_val = sqrt((a-b)^2+c^2);
        if a + b - sr_val < 2 * delta
            k = 2*delta + sr_val - a - b;
            D(i, i) = a + k;
            D(i+1, i+1) = b + k;
        end
        i = i + 2;
    end
end

            
    
    
function [xi, iters] = dl_sr1(X, sample, start, Dhat, ldl_delta, eta, epsilon)
    x = start;
    iters = 0;
    TR_size = Dhat;
    
    [fx, gx] = matern_fn(X, sample, x);
    n = length(start);
    B = eye(n);
    
    xi = {x};
    xval_ctr = 2;
    
    while 1
        if norm(gx) < epsilon
            break
        end
        iters = iters + 1;
        [L, D, p] = modified_ldl(B, ldl_delta);
        
        pk = calculate_pk(L, D, p, gx, TR_size);
        old_fx = fx;
        old_gx = gx;
        
        [fx, gx] = matern_fn(X, sample, (x + pk));
        
        rho_k = (old_fx - fx) / (old_fx - quad_model(old_fx, old_gx, L, D, p, pk));
        B = update_sr1(B, pk, gx - old_gx);

        if rho_k < 0.25
            TR_size = TR_size / 4;
        elseif rho_k > 0.75
            TR_size = min(TR_size * 2, Dhat);
        end
        if rho_k > eta
            x = x + pk;
            xi{1, xval_ctr} = x;
            xval_ctr = xval_ctr + 1;
        else
            fx = old_fx;
            gx = old_gx;
        end
        
    end
    
end

function B1 = update_sr1(B, s, y)
    ymbs = y - B * s;
    if abs(s.' * ymbs) < 1e-8 * norm(s) * norm(ymbs)
        B1 = B;
        return;
    end
    
    B1 = (ymbs * ymbs.') / (ymbs.' * s);
end

function m = quad_model(fx, gx, L, D, p, x)
    m = fx + gx.' * x + x.' * apply_ldl(L, D, p, x) / 2;
end

function pk = calculate_pk(L, D, p, g, TR_size)
    pu = -norm(g)^2 / (g.' * apply_ldl(L, D, p, g)) * g;
    pb = -ldlsolve(L, D, p, g);
    if norm(pu) >= TR_size
        pk = (TR_size / norm(pu)) * pu;
    elseif norm(pb) <= TR_size
        pk = pb;
    else
        n1 = norm(pu);
        n2 = norm(pb - pu);
        cr_p = pb.' * pu - n1^2;
        tau1 = (-(2*cr_p - 2*n2^2) + sqrt((2*cr_p - 2*n2^2)^2 - 4*n2^2*(n1^2 - 2*cr_p + n2^2-TR_size^2))) / (2 * n2^2);
        tau2 = (-(2*cr_p - 2*n2^2) - sqrt((2*cr_p - 2*n2^2)^2 - 4*n2^2*(n1^2 - 2*cr_p + n2^2-TR_size^2))) / (2 * n2^2);
        if (1 <= tau1) && (tau1 <= 2)
            pk = pu + (tau1 - 1)*(pb - pu);
        else
            pk = pu + (tau2 - 1)*(pb - pu);
        end
    end
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

function x = ldlsolve(L, D, p, b)
    b = b(p);
    
    dltx = L\b;
    ltx = D\dltx;
    x = L.'\ltx;
    
    x(p) = x;
end

function y = apply_ldl(L, D, p, x)
    y = x(p);
    y = L.' * y;
    y = D * y;
    y = L * y;
    y(p) = y;
end

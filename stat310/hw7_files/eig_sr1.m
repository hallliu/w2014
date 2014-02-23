function [xi, iters] = eig_sr1(X, sample, start, Dhat, eta, epsilon)
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
        pk = calculate_pk(B, gx);
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


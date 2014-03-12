function [xi, iters] = tr_cg(f, start, Dhat, eta, epsilon)
    x = start;
    iters = 0;
    TR_size = Dhat;
    
    [fx, gx, Hx] = f(x, 2);
    
    xi = {x};
    xval_ctr = 2;
    updated = true;
    
    while 1
        if norm(gx) < epsilon
            break
        end
        iters = iters + 1;
        if updated
            [fx, gx, Hx] = f(x, 2);
            pc_fn = compute_preconditioner(Hx);
        end
        
        pk = pcg_steihaug(fx, gx, Hx, TR_size, pc_fn);
        old_fx = fx;
        
        fx = f(x+pk, 0);
        
        rho_k = (old_fx - fx) / (-gx.' * pk - 0.5*pk.' * Hx * pk);

        if rho_k < 0.25
            TR_size = TR_size / 4;
        elseif rho_k > 0.75
            TR_size = min(TR_size * 2, Dhat);
        end
        if rho_k > eta
            x = x + pk;
            xi{1, xval_ctr} = x;
            xval_ctr = xval_ctr + 1;
            updated = true;
        else
            fx = old_fx;
            updated = false;
        end
        
    end
    
end

function [xi, iters] = eig_sr1(X, sample, start, Dhat, eta, c, epsilon)
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
        pk = calculate_pk(B, gx, c, epsilon, TR_size);
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

function pk = calculate_pk(B, gx, c, epsilon, TR_size)
    [Q, D] = eig(B);
    if D(1,1) < 0
        pk = iterate_pk(Q, D, gx, c, epsilon, TR_size);
        return
    end
    
    % Find indices where D and Q^Tg are zero (or close). If any indices of 
    %D are zero where Q^Tg is not, then there is no solution.
    zero_eigs = (diag(D) < 1e-10);
    qtg = Q.' * gx;
    if ~all(qtg(zero_eigs) < 1e-10)
        pk = iterate_pk(Q, D, gx, c, epsilon, TR_size);
        return
    end
    
    pk = Q * (diag(D) .* qtg);
end

function pk = iterate_pk(Q, D, gx, c, epsilon, TR_size)
    qtg = Q.' * gx;
    eigs = diag(D);
    % This is checking the "hard case" in the book
    l1_inds = eigs - eigs(1) < 1e-8;
    
    if all(abs(qtg(l1_inds)) < 1e-10)
        upper = qtg(~l1_inds);
        lower = eigs(~l1_inds) + eigs(0);
        tau = sqrt(TR_size - sum(upper.^2 / lower.^2));
        pk = sum(upper./lower) .* Q(:, ~l1_inds) + tau * Q(:, 1);
        return
    end
    
    lambda = abs(eigs(1)) * 1.5; % initial value for lambda, kinda arbitrary
    [phi2, dphi2] = phi2_fn(lambda, qtg, eigs, TR_size);
    while abs(phi2) > epsilon
        other_lambda = lambda - phi2 / dphi2;
        if other_lambda <= -eigs(1)
            lambda = -c * eigs(1) + (1-c) * lambda;
        else
            lambda = other_lambda;
        end
        [phi2, dphi2] = phi2_fn(lambda, qtg, eigs, TR_size);
    end
    
    pk = -sum((qtg ./ (eigs + lambda)) .* Q);
end

function [phi2, dphi2] = phi2_fn(lambda, qtg, eigs, TR_size)
    [psq, dp] = pnorm(lambda, qtg, eigs);
    phi2 = 1/TR_size - 1/sqrt(psq);
    dphi2 = dp / psq;
end
function [psq, dp] = pnorm(lambda, qtg, eigs)
    lower = (eigs + lambda).^2;
    psq = sum(qtg.^2 ./ lower);
    lower3 = (eigs + lambda).^3;
    dp = -sum(qtg.^2 ./ lower3) / sqrt(p);
end

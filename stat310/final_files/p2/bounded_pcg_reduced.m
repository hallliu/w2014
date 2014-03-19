% Parameters:
% G, c are the data in the QP problem
% start is the starting point for the CG iteration (the cauchy point)
% actives is a logical array stating which of the lower bound constraints
%   are active
% lower_bnds is a list of lower-bound constraints
% precondition is a flag that tells the CG program whether to precondition

function [x, cg_iters] = bounded_pcg_reduced(G, c, start, actives, lower_bnds, precondition)
    % It's really easy to compute the null space of the constraint matrix:
    % it's just the standard basis vectors corresponding to the inactive
    % constraints.
    i0 = speye(length(start));
    Z = i0(:, ~actives);
    % Similarly, Y is also easy to compute
    Y = i0(:, actives);
    % Finally, since Y=A.', x_Y is just the vector of active constraints
    xy = lower_bnds(actives);
    yxy = Y * xy;
    
    ZGZ = G(~actives, ~actives);
    
    % See what we can do with the preconditioner
    if precondition
        W = ZGZ;
    else
        W = i0(~actives, ~actives);
    end
    
    
    xz = start(~actives);
    
    r = ZGZ * xz + Z.' * G * Y * xy + Z.' * c;
    g = W \ r;
    d = -g;
    
    cg_iters = 0;
    while 1
        cg_iters = cg_iters + 1;
        % I'm pretty sure G is always positive-definite in this problem
        % so there's no need to correct for bad curvature.
        alpha = r.' * g / (d.' * ZGZ * d);
        old_xz = xz;
        xz = xz + alpha * d;
        x = yxy + Z * xz;
        if any(x < lower_bnds)
            x = yxy + Z * old_xz;
            break;
        end
        old_r = r;
        old_g = g;
        
        r = r + alpha * ZGZ * d;
        if norm(r) < 1e-7
            break
        end
        g = W \ r;
        beta = (r.' * g) / (old_r.' * old_g);
        d = -g + beta * d;
    end
    
end

function [ inds, bp_list] = grad_proj(ti)
    import java.util.LinkedList;
    [inds, bp_list] = get_active_bnds(ti);
end

function [cp, active_set] = compute_cauchy_point(x, g, lower_bnds, G, c)
    bps = compute_breakpoints(x, g, lower_bnds);
    [inds, bp_list] = get_active_bnds(bps);
    active_set = false(1, length(g));
    for i = 1:length(bp_list)
        this_ind_activations = inds(i);
        while (this_ind_activations.hasNext())
            active_set(this_ind_activations.pop()) = true;
        end
        cp = find_local_min(x, g, bps, bp_list(i), bp_list(i+1), active_set, G, c);
        if ~isnan(cp)
            break;
        end
    end
end

% Parameters:
% x is the initial starting point
% g is the gradient of the function at that point
% bps is the list of breakpoints, unsorted, associated with each index
% t and t1 are the beginning and end of the subinterval
% actives is a logical array specifying which constraints are active
% G and c are data from the QP problem spec.
function cp = find_local_min(x, g, bps, t, t1, actives, G, c)
    % First compute the p vector
    p = -g;
    p(actives) = 0;
    
    % Compute the location of the path for this breakpoint
    xt = x - t * g;
    clamped_xt = x - bps * g;
    xt(actives) = clamped_xt(actives);
    
    % Compute the two necessary coefficients
    f1 = c.' * p + xt.' * G * p;
    f2 = p.' * G * p;
    
    if f1 < 0
        cp = t;
    elseif -f1 / f2 < t1 - t
        cp = t - f1 / f2;
    else
        cp = NaN;
    end
end

% Computes which components of the gradient become active at any particular
% subinterval.
% inds is an array of linked lists the same length as bp_list which contain at each
% entry the components that become active on the subinterval starting
% at that entry in bp_list.
function [inds, bp_list] = get_active_bnds(ti)
    [bp_list, ~, ic] = unique(ti);
    bp_list = [0 bp_list];
    inds = javaArray('java.util.LinkedList', length(bp_list) - 1);
    
    for i = 1:(length(bp_list) - 1)
        i2 = ic(i);
        inds(i2).push(i);
    end
    
end

function ti = compute_breakpoints(x, g, lower_bnds)
    n = length(x);
    ti = 1:n;
    for i = ti
        if g(i) > 0 && lower_bnds(i) > -Inf
            ti(i) = (x(i) - lower_bnds(i)) / g(i);
            if ti(i) < 1e-8
                ti(i) = 0;
            end
        else
            ti(i) = Inf;
        end
    end
end

function [x, iters, fevals] = p2(f, x0, rho, c_val, m, epsilon)
    import java.util.LinkedList;
    x = x0;
    iters = 0;
    fevals = 0;
    records = LinkedList();
    B = eye(length(x));
    H = eye(length(x));
    
    [fx, gx] = f(x, 1);
    while 1
        if norm(gx, 1) < epsilon
            break
        end
        norm(gx, 1)
        iters = iters + 1;
        
        pk = -compute_Hv(records, gx);
        if isnan(pk(1))
            iters = NaN;
            return;
        end
        
        % Backtracking to find the step size ak
        ak = 1;
        f_val = f(x + ak * pk, 0);
        fevals = fevals + 1;
        while f_val > fx + c_val * ak * (gx.' * pk)
            ak = rho * ak;
            f_val = f(x + ak * pk, 0);
            fevals = fevals + 1;
        end
        old_gx = gx;
        x = x + ak * pk;
        [fx, gx] = f(x, 1);
        
        % Damped bgfs update
        sk = ak * pk;
        yk = gx - old_gx;
        
        bksk = compute_Bv(records, sk);
        if norm(bksk - B * sk) > 1e-10
            bksk;
        end
        
        skyk = sk.' * yk;
        sbs = sk.' * bksk;
        if skyk >= 0.002 * sbs
            theta = 1;
        else
            theta = 0.998 * sbs / (sbs - skyk);
        end
        rk = theta * yk + (1-theta) * bksk;
        records.add({sk, rk, 1/(sk.' * rk), bksk});
        if records.size() > m
            records.remove();
        end
    end
end

% The evaluation function for H_k. It expects records to be stored in a cell array
% in the following order:
% 1: s_k
% 2: r_k
% 3: rho_k
% 4: B_ks_k
% all this is collected into a linked list.
function hv = compute_Hv(records, v)
    if records.size() == 0
        hv = v;
        return;
    end
    import java.util.LinkedList;
    q = v;
    
    up = records.iterator();
    down = records.descendingIterator();
    
    alphas = LinkedList();
    
    while down.hasNext()
        data = cell(down.next());
        alpha = data{3} * (data{1}.' * q);
        q = q - alpha * data{2};
        alphas.push(alpha);
    end
    
    last_data = cell(records.getLast());
    gamma = (last_data{1}.' * last_data{2}) / (last_data{2}.' * last_data{2});
    r = gamma * q;
    
    while up.hasNext()
        data = cell(up.next());
        beta = data{3} * (data{2}.' * r);
        r = r + data{1} * (alphas.pop() - beta);
    end
    hv = r;
end

function bv = compute_Bv(records, v)
    if records.size() == 0
        bv = v;
        return
    end
    
    last_data = cell(records.getLast());
    gamma = (last_data{1}.' * last_data{2}) / (last_data{2}.' * last_data{2});
    bv = v / gamma;
    
    it = records.iterator();
    while it.hasNext()
        data = cell(it.next());
        sk = data{1};
        rk = data{2};
        ck = data{4};
        bv = bv - (ck * (ck.' * v)) / (sk.' * ck) + (rk * (rk.' * v)) / (sk.' * rk);
    end
end

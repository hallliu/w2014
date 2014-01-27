function [dense_r, sparse_r, sizes] = p1(upto)
    sizes = 2.^(1:upto)*50;
    sparse_t = 1:upto; sparse_s = 1:upto;
    dense_t = 1:upto; dense_s = 1:upto;
    
    for i=1:upto
        [dense_t(i), dense_s(i)] = chol_time(sizes(i), 0);
        [sparse_t(i), sparse_s(i)] = chol_time(sizes(i), 1);
    end
    
    d_coefs = polyfit(log(sizes), log(dense_t), 1);
    s_coefs = polyfit(log(sizes), log(sparse_t), 1);
    sprintf('Dense: C=%.3g, alpha=%.3f\n', exp(d_coefs(2)), d_coefs(1))
    sprintf('Sparse: C=%.3g, alpha=%.3f\n', exp(s_coefs(2)), s_coefs(1))

    dense_r = dense_s ./ dense_t;
    sparse_r = sparse_s ./ sparse_t;
end


function [t, s] = chol_time(N, sparse)
    matr = makelap(N, sparse);
    times = 1:5;
    for i=1:5
        tic;
        chol(matr);
        times(i)=toc*1000;
    end
    t = mean(times);
    s = std(times);
end

function lap = makelap(N, sp)
    lap_tmp=2*diag(ones(N,1))-diag(ones(N-1,1),1)-diag(ones(N-1,1),-1);
    if sp
        [i,j,s] = find(lap_tmp);
        lap = sparse(i, j, s, N, N);
    else
        lap = lap_tmp;
    end
end

sizes = [100, 200, 400, 600, 1000, 2000, 3000];
n = length(sizes);
avg_times_pc = zeros(1, n);
avg_times_npc = zeros(1, n);

avg_cg_iters_pc = zeros(1, n);
avg_cg_iters_npc = zeros(1, n);

for i=1:n
    for j=1:3
        sizes(i)
        tic;
        [~, ~, cgi0] = tr_cg(@rosenbrock, randn(sizes(i), 1), 10, 0.2, 1e-7, 1);
        avg_times_pc(i) = avg_times_pc(i) + toc;
        avg_cg_iters_pc(i) = avg_cg_iters_pc(i) + cgi0;
        
        tic;
        [~, ~, cgi1] = tr_cg(@rosenbrock, randn(sizes(i), 1), 10, 0.2, 1e-7, 0);
        avg_times_npc(i) = avg_times_npc(i) + toc;
        avg_cg_iters_npc(i) = avg_cg_iters_npc(i) + cgi1;
    end
end
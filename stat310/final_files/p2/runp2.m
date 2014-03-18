sizes = [4, 8, 16, 32, 64, 100];
times_np = zeros(1, 6);
times_p = zeros(1, 6);
for i = 1:6
    [A, g, f, ~, ~] = obstacle_mihai(sizes(i));
    start = randn(sizes(i)^2, 1);
    for j = 1:3
        tic;
        grad_proj_opt(A, f, g, start, 1e-7, 0);
        times_np(i) = times_np(i) + toc / 3;
        
        tic;
        grad_proj_opt(A, f, g, start, 1e-7, 1);
        times_p(i) = times_p(i) + toc / 3;
    end
end
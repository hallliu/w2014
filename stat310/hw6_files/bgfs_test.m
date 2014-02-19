X = lattice4(3);
y = sample_from_matern(X, [1;1;1;1], 1);
%[xi1, iters1] = dl_bgfs(X, y, [1;2;3;4], 10, 0.2, 1e-6);
[xi2, iters2] = dl_sr1(X, y, [1;2;3;4], 5, 0.1, 0.01, 1e-5);

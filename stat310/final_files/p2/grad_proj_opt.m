function x = grad_proj_opt(G, c, lower_bnds, start, epsilon, pc)
    x = start;
    gx = G*x + c;
    
    while norm(gx) > epsilon
        [cp, actives] = compute_cauchy_point(x, gx, lower_bnds, G, c);
        [x, cg_iters] = bounded_pcg_reduced(G, c, cp, actives, lower_bnds, pc);
        gx = G*x + c;
    end

end


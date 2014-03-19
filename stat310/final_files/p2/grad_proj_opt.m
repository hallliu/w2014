function x = grad_proj_opt(G, c, lower_bnds, start, epsilon, pc)
    x = start;
    gx = G*x + c;
    actives = false(length(gx), 1);
    while 1
        [cp, actives] = compute_cauchy_point(x, gx, lower_bnds, G, c);
        norm(gx(~actives))
        if norm(gx(~actives)) < epsilon
            break
        end
        [x, cg_iters] = bounded_pcg_reduced(G, c, cp, actives, lower_bnds, pc);
        gx = G*x + c;
    end

end

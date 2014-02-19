function [B1, H1] = update_bgfs(B, H, s, y)
    skyk = s.' * y;
    bs = B * s;
    sbs = s.' * bs;
    
    if skyk < 0.2 * sbs
        theta = (0.8 * sbs) / (sbs - skyk);
        r = theta * y - (1 - theta) * bs;
    else
        r = y;
    end
    
%    rho = 1/ (s.' * y);
%    H1 = H - rho * ((H * y) * s.' + s * (y.' * H)) + rho^2 * s * (y.' * H * y) * s.';
%    H1 = H1 + rho * (s * s.');
    
    B1 = B - (bs * bs.') / sbs + (r * r.') / (r.' * s);
    H1 = inv(B1);
end


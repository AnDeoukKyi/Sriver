package company.co.kr.test;

import com.android.volley.Response;
import com.android.volley.toolbox.StringRequest;

import java.util.HashMap;
import java.util.Map;

public class MainRequest extends StringRequest {
    final static private String URL="http://nejoo97.cafe24.com/TEST.php";
    private Map<String,String> parameters;

    public MainRequest(String posX, String posY, Response.Listener<String> listener) {
        super(Method.POST,URL, listener,null);
        parameters = new HashMap<>();
        parameters.put("POSX", posX);
        parameters.put("POSY", posY);

    }

    @Override
    public Map<String, String> getParams() {
        return parameters;
    }
}
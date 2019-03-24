package company.co.kr.sriverforuser;

import com.android.volley.Response;
import com.android.volley.toolbox.StringRequest;

import java.util.HashMap;
import java.util.Map;

public class LoginRequest extends StringRequest {
    final static private String URL="http://nejoo97.cafe24.com/UserLogin.php";
    private Map<String,String> parameters;

    public LoginRequest(String userID, String userPassword, Response.Listener<String> listener) {
        super(Method.POST,URL, listener,null);
        parameters = new HashMap<>();
        parameters.put("ID", userID);
        parameters.put("PW", userPassword);
    }

    @Override
    public Map<String, String> getParams() {
        return parameters;
    }
}
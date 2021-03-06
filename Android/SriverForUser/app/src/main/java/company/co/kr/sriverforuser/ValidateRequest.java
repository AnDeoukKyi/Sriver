package company.co.kr.sriverforuser;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.toolbox.StringRequest;

import java.util.HashMap;
import java.util.Map;

public class ValidateRequest extends StringRequest {
    final static private String URL="http://nejoo97.cafe24.com/UserValidate.php";
    private Map<String,String> parameters;

    public ValidateRequest(String userID, Response.Listener<String> listener) {
        super(Method.POST,URL, listener,null);
        parameters = new HashMap<>();
        parameters.put("ID", userID);
    }

    @Override
    public Map<String, String> getParams() {
        return parameters;
    }
}

package company.co.kr.sriverforuser;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.toolbox.StringRequest;

import java.util.HashMap;
import java.util.Map;

public class RegisterRequest extends StringRequest {
    final static private String URL="http://nejoo97.cafe24.com/UserRegister.php";
    private Map<String,String> parameters;

    public RegisterRequest(String userID, String userPassword, String userEmail, String userCar, String userGender, Response.Listener<String> listener) {
        super(Method.POST,URL, listener,null);
        parameters = new HashMap<>();
        parameters.put("ID", userID);
        parameters.put("PW", userPassword);
        parameters.put("EMAIL", userEmail);
        parameters.put("CAR",userCar);
        parameters.put("GENDER",userGender);
    }

    @Override
    public Map<String, String> getParams() {
        return parameters;
    }
}
